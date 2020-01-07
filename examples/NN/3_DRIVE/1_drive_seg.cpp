/*
* EDDL Library - European Distributed Deep Learning Library.
* Version: 0.3
* copyright (c) 2019, Universidad Politécnica de Valencia (UPV), PRHLT Research Centre
* Date: October 2019
* Author: PRHLT Research Centre, UPV, (rparedes@prhlt.upv.es), (jon@prhlt.upv.es)
* All rights reserved
*/

#include <stdio.h>
#include <stdlib.h>
#include <iostream>

#include "apis/eddl.h"
#include "apis/eddlT.h"

using namespace eddl;

//////////////////////////////////
// Drive segmentation
// https://drive.grand-challenge.org/DRIVE/
//////////////////////////////////
layer SegNet(layer x)
{
    x = ReLu(Conv(x, 64, { 3,3 }, { 1, 1 }, "same"));
    x = ReLu(Conv(x, 64, { 3,3 }, { 1, 1 }, "same"));
    x = MaxPool(x, { 2,2 }, { 2,2 });
    x = ReLu(Conv(x, 128, { 3,3 }, { 1, 1 }, "same"));
    x = ReLu(Conv(x, 128, { 3,3 }, { 1, 1 }, "same"));
    x = MaxPool(x, { 2,2 }, { 2,2 });
    x = ReLu(Conv(x, 256, { 3,3 }, { 1, 1 }, "same"));
    x = ReLu(Conv(x, 256, { 3,3 }, { 1, 1 }, "same"));
    x = ReLu(Conv(x, 256, { 3,3 }, { 1, 1 }, "same"));
    x = MaxPool(x, { 2,2 }, { 2,2 });
    x = ReLu(Conv(x, 512, { 3,3 }, { 1, 1 }, "same"));
    x = ReLu(Conv(x, 512, { 3,3 }, { 1, 1 }, "same"));
    x = ReLu(Conv(x, 512, { 3,3 }, { 1, 1 }, "same"));
    x = MaxPool(x, { 2,2 }, { 2,2 });
    x = ReLu(Conv(x, 512, { 3,3 }, { 1, 1 }, "same"));
    x = ReLu(Conv(x, 512, { 3,3 }, { 1, 1 }, "same"));
    x = ReLu(Conv(x, 512, { 3,3 }, { 1, 1 }, "same"));
    x = MaxPool(x, { 2,2 }, { 2,2 });

    x = UpSampling(x, { 2,2 });
    x = ReLu(Conv(x, 512, { 3,3 }, { 1, 1 }, "same"));
    x = ReLu(Conv(x, 512, { 3,3 }, { 1, 1 }, "same"));
    x = ReLu(Conv(x, 512, { 3,3 }, { 1, 1 }, "same"));
    x = UpSampling(x, { 2,2 });
    x = ReLu(Conv(x, 512, { 3,3 }, { 1, 1 }, "same"));
    x = ReLu(Conv(x, 512, { 3,3 }, { 1, 1 }, "same"));
    x = ReLu(Conv(x, 256, { 3,3 }, { 1, 1 }, "same"));
    x = UpSampling(x, { 2,2 });
    x = ReLu(Conv(x, 256, { 3,3 }, { 1, 1 }, "same"));
    x = ReLu(Conv(x, 256, { 3,3 }, { 1, 1 }, "same"));
    x = ReLu(Conv(x, 128, { 3,3 }, { 1, 1 }, "same"));
    x = UpSampling(x, { 2,2 });
    x = ReLu(Conv(x, 128, { 3,3 }, { 1, 1 }, "same"));
    x = ReLu(Conv(x, 64, { 3,3 }, { 1, 1 }, "same"));
    x = UpSampling(x, { 2,2 });
    x = ReLu(Conv(x, 64, { 3,3 }, { 1, 1 }, "same"));
    x = Conv(x, 1, { 3,3 }, { 1,1 }, "same");

    return x;
}

int main(int argc, char **argv){

  // Settings
  int epochs = 25;
  int batch_size =1;

  // network for Data Augmentation
  layer in1=Input({3,584,565});
  layer in2=Input({1,584,565});

  layer l=Concat({in1,in2});
  l=Crop(l,{512,512});
  l=CropScaleRandom(l, {0.8f, 1.0f});
  layer img=Select(l,{"0:3"});
  layer mask=Select(l,{"3"});

  // Define DA model inputs
  model danet=Model({in1,in2},{});
  // Build model for DA
  build(net_da);

  summary(net_da);

  // Perform DA in GPU
  //toGPU(net_da);


  // Build SegNet
  layer in=Input({3,512,512});
  layer out=SegNet(in);
  model segnet=Model({in},{out});
  build(net,
    sgd(0.01, 0.9), // Optimizer
    {"mse"}, // Losses
    {"mse"} // Metrics
  );
  //toGPU(net);
  summary(segnet);

  // Load and preprocess training data
  cout<<"Reading train numpy\n";
  tensor x_train_f = Tensor::load<unsigned char>("x_train.npy");
  x_train_f->info();

  tensor x_train=Tensor::permute(x_train_f, {0,3,1,2});
  x_train->info();
  //permute

  cout<<"Reading test numpy\n";
  tensor y_train = Tensor::load<unsigned char>("y_train.npy");
  y_train->info();
  eddlT::reshape_(y_train,{20,1,584,565});
  y_train->info();

  tensor xbatch = eddlT::create({batch_size,3,584,565});
  tensor ybatch = eddlT::create({batch_size,1,584,565});

  int num_batches=4;
  for(int i=0;i<epochs;i++) {
    for(int j=0;j<num_batches;j++)  {

      next_batch({x_train,y_train},{xbatch,ybatch});

      // DA
      forward(net_da, {xbatch,ybatch});

      // get tensors from DA
      tensor xbatch_da = getTensor(img);
      xbatch_da->info();
      tensor ybatch_da = getTensor(mask);
      ybatch_da->info();
      // SegNet
      train_batch(net, {xbatch_da},{ybatch_da});

      print_loss(net,j);
      printf("\r");
    }
    printf("\n");
  }

}
