// This file is part of EDDLL an European Distributed Deep Learning Library.
// Developed within the DeepHealth project.
// Boosting AI in Europe.
//
// The MIT License (MIT)
//
// Copyright (c) 2019
// 	     Roberto Paredes Palacios, <rparedes@dsic.upv.es>
// 	     Jon Ander Gómez, <jon@dsic.upv.es>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include <stdio.h>
#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <chrono>
#include <thread>
#include "net.h"

#ifdef cGPU
#include "gpu/tensor_cuda.h"
#endif

using namespace std;
using namespace std::chrono;

ostream& operator<<(ostream& os, const shape s)
{
    int i;
    os << "(";
    for (i = 0; i < s.size()-1; ++i)
    {
        os << s[i];
        os <<"x";
    }
    os<<s[i]<<")";

    return os;
}


struct tdata
{
    Net *net;
    vtensor Xt;
    vtensor Yt;
    int batch;
};

////////////////////////////////////
///// BASE NET CLASS
////////////////////////////////////

Net::Net(const initializer_list<Layer*>& in,const initializer_list<Layer*>& out):Net(vlayer(in.begin(), in.end()),vlayer(out.begin(), out.end())){}

/////////////////////////////////////////

Net::Net(vlayer in,vlayer out)
{
    optimizer=NULL;
    lin=in;
    lout=out;
    for(int i=0;i<lin.size();i++)
        walk(lin[i]);
}


/////////////////////////////////////////
int Net::inNet(Layer *l)
{
    for(int i = 0; i != layers.size(); i++)
        if (l==layers[i]) return 1;
    return 0;
}


/////////////////////////////////////////
void Net::walk(Layer *l)
{
    if (!inNet(l))
    {
        layers.push_back(l);
        for(int i = 0; i != l->child.size(); i++)
            walk(l->child[i]);
    }
}


Layer *Net::getLayer(string name)
{
    for(int i = 0; i != layers.size(); i++)
        if (name==layers[i]->name) return layers[i];

    msg("layer %s not found","Net.getLayer");
    return NULL;
}


void Net::info()
{

    for(int i=0;i<layers.size();i++)
        fprintf(stderr,"%s ",layers[i]->name.c_str());

    cout<<"\n";
    for(int i=0;i<layers.size();i++)
    {
        cout<<layers[i]->name<<": ";
        shape si=layers[i]->input->getshape();
        shape so=layers[i]->output->getshape();
        cout<<si<<"-->"<<so<<"\n";
    }

    fprintf(stderr,"\n");

}


/////////////////////////////////////////
void Net::initialize()
{
    for(int i = 0; i != layers.size(); i++)
        layers[i]->initialize();
}


/////////////////////////////////////////
void Net::reset()
{
    for(int i = 0; i != layers.size(); i++)
        layers[i]->reset();
}


/////////////////////////////////////////
void Net::fts()
{
    int i,j,k,n;
    vector<int> visit;
    vector<int> gin;

    fprintf(stderr,"FTS:");
    for(i=0;i<layers.size();i++)
    {
        visit.push_back(0);
        gin.push_back(layers[i]->lin);
    }

    for(i=0;i<layers.size();i++)
    {

        for(j=0;j<layers.size();j++)
            if ((gin[j]==0)&&(!visit[j])) break;

        if (j==layers.size())
            msg("error recurrent net","Net.fts");

        if (layers[j]->lout)
            fprintf(stderr,"%s-->",layers[j]->name.c_str());
        else
            fprintf(stderr,"%s |",layers[j]->name.c_str());

        visit[j]=1;
        vfts.push_back(layers[j]);

        for(k=0;k<layers[j]->lout;k++)
            for(n=0;n<layers.size();n++)
                if(layers[n]==layers[j]->child[k]) gin[n]--;

    }
    fprintf(stderr,"\n");

}


/////////////////////////////////////////
void Net::bts()
{
    int i,j,k,n;
    vector<int> visit;
    vector<int> gout;

    fprintf(stderr,"BTS:");
    for(i=0;i<layers.size();i++)
    {
        visit.push_back(0);
        gout.push_back(layers[i]->lout);
    }

    for(i=0;i<layers.size();i++)
    {

        for(j=0;j<layers.size();j++)
            if ((gout[j]==0)&&(!visit[j])) break;

        if (j==layers.size())
            msg("error recurrent net in","Net.bts");

        if (layers[j]->lin)
            fprintf(stderr,"%s-->",layers[j]->name.c_str());
        else
            fprintf(stderr,"%s |",layers[j]->name.c_str());

        visit[j]=1;
        vbts.push_back(layers[j]);

        for(k=0;k<layers[j]->lin;k++)
            for(n=0;n<layers.size();n++)
                if(layers[n]==layers[j]->parent[k]) gout[n]--;

    }
    fprintf(stderr,"\n");
}


/////////////////////////////////////////
int isIn(Layer *l,vlayer vl,int &ind)
{
    for(int i=0;i<vl.size();i++)
        if(l==vl[i]) {ind=i;return 1;}

        return 0;
}

/////////////////////////////////////////
int isInorig(Layer *l,vlayer vl,int &ind)
{
    for(int i=0;i<vl.size();i++)
        if(l==vl[i]->orig) {ind=i;return 1;}

        return 0;
}


/////////////////////////////////////////
void Net::build(optim *opt,const initializer_list<string>& c,const initializer_list<string>& m) {
  build(opt,c,m,DEV_CPU);
}
void Net::build(optim *opt,const initializer_list<string>& c,const initializer_list<string>& m,int todev)
{
    vstring co=vstring(c.begin(), c.end());
    vstring me=vstring(m.begin(), m.end());

    build(opt,co,me);


    if (todev==DEV_CPU) {
      if (dev==DEV_CPU) {
        // split on multiple threads
        unsigned int nthreads = std::thread::hardware_concurrency();
        cout<<"set threads to "<<nthreads<<"\n";
        if (nthreads>1)
          split(nthreads,DEV_CPU);
      }
    }
    else{
      if (dev<DEV_FPGA) {
        #ifndef cGPU
         msg("EDDLL not compiled for GPU","Net.build");
        #else
         // split on multiple GPUs
         int ngpus=gpu_devices();
         cout<<"split into "<<ngpus<<" GPUs devices\n";
         split(ngpus,DEV_GPU);
        #endif
      }
      else {
        // split on multiple FPGAs
      }
    }
}


void Net::build(optim *opt,vstring co,vstring me)
{

    fprintf(stderr,"Build net\n");
    if (co.size()!=lout.size())
        msg("Loss list size does not match output list","Net.build");

    if (co.size()!=lout.size())
        msg("Metric list size does not match output list" ,"Net.build");

   // check devices
   dev=-1;
   int ind;
   for(int i=0;i<layers.size();i++)
      // do not consider input layers, since they are always on CPU
      if (!isIn(layers[i],lin,ind)) {
        if (dev==-1) dev=layers[i]->dev;
        else {
          if (layers[i]->dev!=dev)
            msg("Net with layers in different devicess" ,"Net.build");
        }
      }
    if (dev==DEV_CPU)
      cout<<"Net running on CPU\n";
    else if (dev<DEV_FPGA)
      cout<<"Net running on GPU "<<dev-DEV_GPU<<"\n";
    else
      cout<<"Net running on FPGA "<<dev-DEV_FPGA<<"\n";

// set optimizer
    optimizer=opt;
    optimizer->setlayers(layers);
// Initialize fiting errors vector
    for(int i=0;i<co.size();i++)
    {
        strcosts.push_back(co[i]);
        fiterr.push_back(0.0);
        fiterr.push_back(0.0);
    }
// set loss functions and create targets tensors
    for(int i=0;i<co.size();i++)
    {
        losses.push_back(new Loss(co[i]));
        if (co[i]=="soft_cent") lout[i]->delta_bp=1;
        lout[i]->target=new Tensor(lout[i]->output->getshape(),dev);
    }

// set metrics
    for(int i=0;i<me.size();i++)
    {
        strmetrics.push_back(me[i]);
        metrics.push_back(new Metric(me[i]));

    }

// forward sort
    fts();
// backward sort
    bts();
// random params
    initialize();

}


/////////////////////////////////////
void Net::split(int c,int todev)
{
    int i,j,k,l;

    vlayer nlayers;
    vlayer nin;
    vlayer nout;
    Layer *p;
    int ind;


    for(i=0;i<c;i++)
    {
        cout<<"Split "<<i<<"\n";

        nlayers.clear();
        nin.clear();
        nout.clear();

// set inputs
        for(j=0;j<lin.size();j++)
        {
            vlayer par;
            if (todev==DEV_CPU) nin.push_back(layers[j]->share(c,par));
            else nin.push_back(layers[j]->clone(c,par,todev+i));
            nlayers.push_back(nin[j]);
        }
        for(k=0;k<layers.size();k++)
            for(j=0;j<layers.size();j++)
        {
            if (!isInorig(layers[j],nlayers,ind))
            {
                vlayer par;
                for(l=0;l<layers[j]->parent.size();l++)
                    if (!isInorig(layers[j]->parent[l],nlayers,ind)) break;
                    else par.push_back(nlayers[ind]);

                if (l==layers[j]->parent.size())
                    if (todev==DEV_CPU) nlayers.push_back(layers[j]->share(i,par));
                    else nlayers.push_back(layers[j]->clone(i,par,todev+i));
            }
        }

// set outputs
        for(j=0;j<lout.size();j++)
            if (isInorig(lout[j],nlayers,ind))
                nout.push_back(nlayers[ind]);

// create new net
        snets.push_back(new Net(nin,nout));

// build new net
        snets[i]->build(optimizer->clone(),strcosts,strmetrics);
    }

}


/////////////////////////////////////////
void Net::forward()
{
    for(int i=0;i<vfts.size();i++) {
        vfts[i]->forward();
      }
}


void Net::delta()
{
    for(int i=0;i<lout.size();i++)
        losses[i]->delta(lout[i]->target,lout[i]->output,lout[i]->delta);
}


void Net::loss()
{
    int p=0;
    for(int i=0;i<lout.size();i++,p+=2)
    {
// loss value
        fiterr[p]=losses[i]->value(lout[i]->target,lout[i]->output);
// metric value
        fiterr[p+1]=metrics[i]->value(lout[i]->target,lout[i]->output);
    }
}


/////////////////////////////////////////
void Net::backward()
{
    for(int i=0;i<vbts.size();i++)
        vbts[i]->backward();
}


/////////////////////////////////////////
void Net::applygrads(int batch)
{
    optimizer->applygrads(batch);
}


/////////////////////////////////////////
void Net::fit(const initializer_list<Tensor*>& in,const initializer_list<Tensor*>& out,int batch, int epochs)
{
    int i,j,k,l,n;

    if (optimizer==NULL)
        msg("Net is not build","Net.fit");

    vtensor tin=vtensor(in.begin(), in.end());
    vtensor tout=vtensor(out.begin(), out.end());

// Check list sizes
    if (tin.size()!=lin.size())
        msg("input tensor list does not match with defined input layers","Net.fit");
    if (tout.size()!=lout.size())
        msg("output tensor list does not match with defined output layers","Net.fit");

// Check data consistency
    n=tin[0]->sizes[0];
    for(i=1;i<tin.size();i++)
        if(tin[i]->sizes[0]!=n)
            msg("different number of samples in input tensor","Net.fit");

    for(i=1;i<tout.size();i++)
        if(tout[i]->sizes[0]!=n)
            msg("different number of samples in output tensor","Net.fit");

// Create internal variables
// Input and output batch...
    vtensor X;
    for(i=0;i<tin.size();i++)
    {
        shape s=tin[i]->getshape();
        s[0]=batch;
        X.push_back(new Tensor(s));
    }
    for(i=0;i<n;i++)
        ind.push_back(i);
    for(i=0;i<batch;i++)
        sind.push_back(0);

    vtensor Y;
    verr errors;
    for(i=0;i<tout.size();i++)
    {
        shape s=tout[i]->getshape();
        s[0]=batch;
        Y.push_back(new Tensor(s));
        errors.push_back(0.0);
        errors.push_back(0.0);
    }

// Check sizes w.r.t layers in and out
    for(int i=0;i<lin.size();i++)
        if (!Tensor::eqsize(lin[i]->input,X[i]))
            msg("input tensor shapes does not match","Net.fit");

    for(int i=0;i<lout.size();i++)
        if (!Tensor::eqsize(lout[i]->output,Y[i]))
            msg("output tensor shapes does not match","Net.fit");

// Start training
    fprintf(stderr,"%d epochs of %d batches of size %d\n",epochs,n/batch,batch);
    for(i=0;i<epochs;i++)
    {
        high_resolution_clock::time_point e1 = high_resolution_clock::now();
        fprintf(stderr,"Epoch %d\n",i+1);
        for(j=0;j<2*tout.size();j++) errors[j]=0.0;

        for(j=0;j<n/batch;j++)
        {
// random batches
            for(k=0;k<batch;k++)
                sind[k]=rand()%n;

// copy a batch from tin--> X
            for(k=0;k<lin.size();k++)
                Tensor::select(tin[k],X[k],sind);
// copy a batch from tout--> Y
            for(k=0;k<lout.size();k++)
                Tensor::select(tout[k],Y[k],sind);

            high_resolution_clock::time_point t1 = high_resolution_clock::now();

            train_batch(X,Y);

            high_resolution_clock::time_point t2 = high_resolution_clock::now();

            duration<double> time_span = t2 - t1;

            int p=0;
            fprintf(stderr,"batch %d ",j+1);
            for(k=0;k<tout.size();k++,p+=2)
            {
                errors[p]+=fiterr[p];
                errors[p+1]+=fiterr[p+1];
                fprintf(stderr,"%s(%s=%1.3f,%s=%1.3f) ",lout[k]->name.c_str(),losses[k]->name.c_str(),errors[p]/(batch*(j+1)),metrics[k]->name.c_str(),errors[p+1]/(batch*(j+1)));
                fiterr[p]=fiterr[p+1]=0.0;
            }
            fprintf(stderr,"%1.3f secs/batch\r",time_span.count());

        }
        high_resolution_clock::time_point e2 = high_resolution_clock::now();
        duration<double> epoch_time_span = e2 - e1;

        fprintf(stderr,"\n%1.3f secs/epoch\n",epoch_time_span);
    }
}


/////////////////////////////////////////
void Net::train_batch(const initializer_list<Tensor*>& in,const initializer_list<Tensor*>& out)
{
    int i,j,n;

    vtensor X=vtensor(in.begin(), in.end());
    vtensor Y=vtensor(out.begin(), out.end());

// Check sizes
    if (X.size()!=lin.size())
        msg("input tensor list does not match","Net.train_batch");
    if (Y.size()!=lout.size())
        msg("output tensor list does not match","Net.train_batch");

    for(int i=0;i<lin.size();i++)
        if (!Tensor::eqsize(lin[i]->input,X[i]))
            msg("input tensor shapes does not match","Net.train_batch");

    for(int i=0;i<lin.size();i++)
        if (!Tensor::eqsize(lout[i]->output,Y[i]))
            msg("output tensor shapes does not match","Net.train_batch");

    train_batch(X,Y);
}


/////////////////////////////////////////
void Net::train_batch(vtensor X, vtensor Y)
{
    int i,j;


    if (snets.size()<2) // One CPU thread __OR__ One {GPU,FPGA}
    {
        // This copies goes from CPU to CPU or {GPU,FPGA}
        if (snets.size()) { // one {GPU,FPGA}
          // With only one device it is not necessary
          // to synchronize params

          // this copy is between cpu memory and {GPU,FPGA} memory
          for(i=0;i<X.size();i++)
              Tensor::copy(X[i],snets[0]->lin[i]->input);
          for(i=0;i<Y.size();i++)
              Tensor::copy(Y[i],lout[i]->target);

          snets[0]->reset();
          snets[0]->forward();
          snets[0]->delta();
          snets[0]->loss();
          snets[0]->backward();
          snets[0]->applygrads(X[0]->sizes[0]);
        }
        else { //one CPU thread
          // this copy is between cpu memory
          for(i=0;i<X.size();i++)
              Tensor::copy(X[i],lin[i]->input);
          for(i=0;i<Y.size();i++)
              Tensor::copy(Y[i],lout[i]->target);

          reset();
          forward();
          delta();
          loss();
          backward();
      }
    }
    else // multiple CPU_cores or GPUs or FPGAs
    {
        // In case of multiple GPUS or FPGA
        // it is necessary to synchronize params
        void *status;
        int rc;
        pthread_t thr[100];
        struct tdata td[100];

        int batch=X[0]->sizes[0]/snets.size();
        vind sind;
        for(int i=0;i<batch;i++)
            sind.push_back(0);

        vtensor Xs[snets.size()];
        for(int i=0;i<snets.size();i++)
            for(int j=0;j<X.size();j++)
        {
            shape s=X[0]->getshape();
            s[0]=batch;
            Xs[i].push_back(new Tensor(s));
        }

        vtensor Ys[snets.size()];
        for(int i=0;i<snets.size();i++)
            for(int j=0;j<Y.size();j++)
        {
            shape s=Y[0]->getshape();
            s[0]=batch;
            Ys[i].push_back(new Tensor(s));
        }

        for(int i=0;i<snets.size();i++)
        {

            for(int j=0;j<batch;j++)
                sind[j]=(i*batch)+j;

            for(int j=0;j<X.size();j++)
                Tensor::select(X[j],Xs[i][j],sind);

            for(int j=0;j<Y.size();j++)
                Tensor::select(Y[j],Ys[i][j],sind);

//thread params
            td[i].net=snets[i];
            td[i].batch=X[0]->sizes[0];
            td[i].Xt.clear();
            for(int j=0;j<Xs[i].size();j++)
                td[i].Xt.push_back(Xs[i][j]);

            td[i].Yt.clear();
            for(int j=0;j<Ys[i].size();j++)
                td[i].Yt.push_back(Ys[i][j]);

//call thread
            rc = pthread_create(&thr[i], NULL,train_batch_t, (void *)(&td[i]));

            if (rc)
            {
                fprintf(stderr,"Error:unable to create thread %d",rc);
                exit(-1);
            }
        }

        for(int i=0;i<snets.size();i++)
        {
            rc = pthread_join(thr[i], &status);
            if (rc)
            {
                cout << "Error:unable to join," << rc << endl;
                exit(-1);
            }
        }


        if (snets[i]->dev==DEV_CPU) {
          for(int i=0;i<snets.size();i++)
          {
  //call thread
                rc = pthread_create(&thr[i], NULL,applygrads_t, (void *)(&td[i]));

              if (rc)
              {
                    fprintf(stderr,"Error:unable to create thread %d",rc);
                  exit(-1);
              }
          }

            for(int i=0;i<snets.size();i++)
          {
              rc = pthread_join(thr[i], &status);
              if (rc)
                {
                  cout << "Error:unable to join," << rc << endl;
                  exit(-1);
              }
          }
        }

        for(int i=0;i<snets.size();i++)
        {
            for(int j=0;j<2*lout.size();j++)
            {
                fiterr[j]+=snets[i]->fiterr[j];
            }
        }

        // In case of GPUS or FPGA synchronize params
        if (snets[0]->dev!=DEV_CPU){
          for(int j;j<layers.size();j++)
            for(int k=0;k<layers[j]->params.size();k++) {
              // Taking average
              layers[j]->params[k]->set(0.0);
              for(int i=0;i<snets.size();i++) {
                Tensor::inc(snets[i]->layers[j]->params[k],layers[j]->params[k]);
              }
              layers[j]->params[k]->div(snets.size());

              // copy-back to devices
              for(int i=0;i<snets.size();i++) {
                Tensor::copy(layers[j]->params[k],snets[i]->layers[j]->params[k]);
              }
            }
        }
        /////////////////////////////////////////////////

        for(int i=0;i<snets.size();i++)
            for(int j=0;j<X.size();j++)
                delete Xs[i][j];

        for(int i=0;i<snets.size();i++)
            for(int j=0;j<Y.size();j++)
                delete Ys[i][j];


    }
}


/////////////////////////////////////////
void *train_batch_t(void *t)
{
    int i,j;
    tdata *targs=(tdata *)t;

    Net *net=targs->net;

// these copies can go from CPU to {CPU,GPU,FPGA}
    for(i=0;i<targs->Xt.size();i++)
        Tensor::copy(targs->Xt[i],net->lin[i]->input);
    for(i=0;i<targs->Yt.size();i++)
        Tensor::copy(targs->Yt[i],net->lout[i]->target);

    net->reset();
    net->forward();
    net->delta();
    net->loss();
    net->backward();
    if (net->dev>DEV_CPU) net->applygrads(targs->batch);

}


/////////////////////////////////////////
void *applygrads_t(void *t)
{
    int i,j;
    tdata *targs=(tdata *)t;

    Net *net=targs->net;

    net->applygrads(targs->batch);

}


///////////////////////////////////////////

//////
