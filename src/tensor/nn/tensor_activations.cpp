/*
* EDDL Library - European Distributed Deep Learning Library.
* Version: 0.2
* copyright (c) 2019, Universidad Politécnica de Valencia (UPV), PRHLT Research Centre
* Date: October 2019
* Author: PRHLT Research Centre, UPV, (rparedes@prhlt.upv.es), (jon@prhlt.upv.es)
* All rights reserved
*/
#include "tensor_nn.h"
#include "../../hardware/cpu/nn/cpu_nn.h"

#ifdef cGPU
#include "../../hardware/gpu/gpu_tensor.h"
#include "../../hardware/gpu/gpu_hw.h"
#include "../../hardware/gpu/nn/gpu_nn.h"
#endif

#ifdef cFPGA
#include "../../hardware/fpga/tensor_hls_op.h"
#endif

ProfilerStorage ReLu_ps("ReLu");
ProfilerStorage D_ReLu_ps("D_ReLu");
ProfilerStorage Softmax_ps("Softmax");

// ReLU
void ReLu(Tensor *A, Tensor *B) {
    BlockProfiler prof_(ReLu_ps);

    if (A->device != B->device) msg("Tensors in different devices", "Tensor::ReLu");
    if (!Tensor::eqsize(A, B)) msg("Incompatible dims", "Tensor::ReLu");

    B->tsem->lock();
    if (A->isCPU()) {
        //printf("RELU:CPU\n");
        cpu_relu(A, B);
    }
#ifdef cGPU
    else if (A->isGPU())
      {
      gpu_relu(A,B);
      }
#endif
#ifdef cFPGA
    else if (A->isFPGA())
     {
     fpga_tensor_operation(A, B, FPGARELU);
     //printf("FPGA::RELU\n");
     // Tensor *nA=new Tensor(A->getShape(),DEV_CPU);
     // Tensor *nB=new Tensor(B->getShape(),DEV_CPU);
     // fpga_copy_from_fpga(A, nA->ptr);
     // fpga_copy_from_fpga(B, nB->ptr);
     // cpu_relu(nA, nB);
     // fpga_copy_to_fpga(nA->ptr, A);
     // fpga_copy_to_fpga(nB->ptr, B);
     // delete nA;
     // delete nB;
     }
#endif

    B->tsem->unlock();
}

// RELU Derivative, always increment over parent delta
void D_ReLu(Tensor *D, Tensor *I, Tensor *PD) {

    BlockProfiler prof_(D_ReLu_ps);
    if ((D->device != I->device) || (D->device != PD->device)) msg("Tensors in different devices", "Tensor::D_ReLu");
    if ((!Tensor::eqsize(D, I)) || (!Tensor::eqsize(D, PD))) msg("Incompatible dims", "Tensor::D_ReLu");

    PD->tsem->lock();
    if (D->isCPU()) {
        cpu_d_relu(D, I, PD);
    }
#ifdef cGPU
    else if (D->isGPU())
      {
        gpu_d_relu(D,I,PD);

      }
#endif
#ifdef cFPGA
    else if (D->isFPGA())
      {
        fpga_relu_soft_d(D, I, PD, FPGARELU);
        //printf("FPGA::D_RELU\n");
        // printf("DRELU:CPU parte FPGA\n");
        // Tensor *nD=new Tensor(D->getShape(),DEV_CPU);
        // Tensor *nI=new Tensor(I->getShape(),DEV_CPU);
        // Tensor *nPD=new Tensor(PD->getShape(),DEV_CPU);
        // fpga_copy_from_fpga(D, nD->ptr);
        // fpga_copy_from_fpga(I, nI->ptr);
        // fpga_copy_from_fpga(PD, nPD->ptr);
        // cpu_d_relu(nD, nI, nPD);
        // fpga_copy_to_fpga(nD->ptr, D);
        // fpga_copy_to_fpga(nI->ptr, I);
        // fpga_copy_to_fpga(nPD->ptr, PD);
        // delete nD;
        // delete nI;
        // delete nPD;
      }
#endif
    PD->tsem->unlock();
}

// LeakyReLU
void LReLu(Tensor *A, Tensor *B,float param) {
    if (A->device != B->device) msg("Tensors in different devices", "Tensor::ReLu");
    if (!Tensor::eqsize(A, B)) msg("Incompatible dims", "Tensor::ReLu");

    B->tsem->lock();
    if (A->isCPU()) {
        cpu_lrelu(A, B,param);
    }
#ifdef cGPU
    else if (A->isGPU())
      {
      gpu_lrelu(A,B,param);
      }
#endif
#ifdef cFPGA
    else {

    }
#endif

    B->tsem->unlock();
}

// RELU Derivative, always increment over parent delta
void D_LReLu(Tensor *D, Tensor *I, Tensor *PD,float param) {
    if ((D->device != I->device) || (D->device != PD->device)) msg("Tensors in different devices", "Tensor::D_ReLu");
    if ((!Tensor::eqsize(D, I)) || (!Tensor::eqsize(D, PD))) msg("Incompatible dims", "Tensor::D_ReLu");

    PD->tsem->lock();
    if (D->isCPU()) {
        cpu_d_lrelu(D, I, PD,param);
    }
#ifdef cGPU
    else if (D->isGPU())
      {
        gpu_d_lrelu(D,I,PD,param);

      }
#endif
#ifdef cFPGA
    else {

    }
#endif
    PD->tsem->unlock();
}


// Sigmoid
void Sigmoid(Tensor *A, Tensor *B) {
    if (A->device != B->device) msg("Tensors in different devices", "Tensor::Sigmoid");
    if (!Tensor::eqsize(A, B)) msg("Incompatible dims", "Tensor::Sigmoid");

    B->tsem->lock();
    if (A->isCPU()) {
        cpu_sigmoid(A, B);
    }
#ifdef cGPU
    else if (A->isGPU())
      {
      gpu_sigmoid(A,B);
      }
#endif
#ifdef cFPGA
    else {
	msg("Sigmoid Not implemented in FPGA yet\n");
    }
#endif

    B->tsem->unlock();
}

// Sigmoid Derivative, always increment over parent delta
void D_Sigmoid(Tensor *D, Tensor *I, Tensor *PD) {
    if ((D->device != I->device) || (D->device != PD->device)) msg("Tensors in different devices", "Tensor::D_Sigmoid");
    if ((!Tensor::eqsize(D, I)) || (!Tensor::eqsize(D, PD))) msg("Incompatible dims", "Tensor::D_Sigmoid");

    PD->tsem->lock();
    if (D->isCPU()) {
        cpu_d_sigmoid(D, I, PD);
    }
#ifdef cGPU
    else if (D->isGPU())
      {
        gpu_d_sigmoid(D,I,PD);

      }
#endif
#ifdef cFPGA
    else {
        msg("Sigmoid Not implemented in FPGA yet\n");
    }
#endif
    PD->tsem->unlock();
}

// Sigmoid
void Tanh(Tensor *A, Tensor *B) {
    if (A->device != B->device) msg("Tensors in different devices", "Tensor::Tanh");
    if (!Tensor::eqsize(A, B)) msg("Incompatible dims", "Tensor::Tanh");

    B->tsem->lock();
    if (A->isCPU()) {
        cpu_tanh(A, B);
    }
#ifdef cGPU
    else if (A->isGPU())
      {
      gpu_tanh(A,B);
      }
#endif
#ifdef cFPGA
    else {

    }
#endif

    B->tsem->unlock();
}

// Sigmoid Derivative, always increment over parent delta
void D_Tanh(Tensor *D, Tensor *I, Tensor *PD) {
    if ((D->device != I->device) || (D->device != PD->device)) msg("Tensors in different devices", "Tensor::D_Sigmoid");
    if ((!Tensor::eqsize(D, I)) || (!Tensor::eqsize(D, PD))) msg("Incompatible dims", "Tensor::D_Sigmoid");

    PD->tsem->lock();
    if (D->isCPU()) {
        cpu_d_tanh(D, I, PD);
    }
#ifdef cGPU
    else if (D->isGPU())
      {
        gpu_d_tanh(D,I,PD);

      }
#endif
#ifdef cFPGA
    else {

    }
#endif
    PD->tsem->unlock();
}


// SOFTMAX
void Softmax(Tensor *A, Tensor *B) {

    BlockProfiler prof_(Softmax_ps);
    if (A->device != B->device) msg("Tensors in different devices", "Tensor::Softmax");
    if (!Tensor::eqsize(A, B)) msg("Incompatible dims", "Tensor::Softmax");
    if (A->ndim != 2) msg("Softmax only over 2D Tensor (batch x logits)", "Tensor::Softmax");

    B->tsem->lock();

    if (A->isCPU()) {
        cpu_softmax(A, B);
    }
#ifdef cGPU
    else if (A->isGPU())
      {
        gpu_softmax(A,B);
      }
#endif
#ifdef cFPGA
    else if (A->isFPGA())
      {
        fpga_tensor_operation(A, B, FPGASOFTM);
        // printf("A shape: \n");
        // printf("A shape 0: %d\n",A->shape[0]);
        // printf("A shape 1: %d\n",A->shape[1]);
        // printf("B shape: \n");
        // printf("B shape 0: %d\n",B->shape[0]);
        // printf("B shape 1: %d\n",B->shape[1]);
        // printf("FPGA::SOFTMAX\n");
        // Tensor *nA=new Tensor(A->getShape(),DEV_CPU);
        // Tensor *nB=new Tensor(B->getShape(),DEV_CPU);
        // fpga_copy_from_fpga(A, nA->ptr);
        // fpga_copy_from_fpga(B, nB->ptr);
        // cpu_softmax(nA, nB);
        // fpga_copy_to_fpga(nA->ptr, A);
        // fpga_copy_to_fpga(nB->ptr, B);
        // delete nA;
        // delete nB;
      }
#endif

    B->tsem->unlock();
}

// SOFTMAX DERIVATIVE
void D_Softmax(Tensor *D, Tensor *I, Tensor *PD) {
    if ((D->device != I->device) || (D->device != PD->device)) msg("Tensors in different devices", "Tensor::D_Softmax");
    if ((!Tensor::eqsize(D, I)) || (!Tensor::eqsize(D, PD))) msg("Incompatible dims", "Tensor::D_Softmax");
    if (D->ndim != 2) msg("D_Softmax only over 2D Tensor (batch x delta_probs)", "Tensor::D_Softmax");

    if (D->isCPU()) {
       cpu_d_softmax(D, I, PD);
    }
#ifdef cGPU
    else if (D->isGPU())
      {

        Tensor *aux=new Tensor(D->getShape(),D->device);
        aux->fill_(1.0);
        Tensor::add(1.0,aux,-1.0,I,aux,0);
        Tensor::el_mult(I,aux,aux,0);
        Tensor::el_mult(D,aux,PD,1);

        delete aux;
      }
#endif
#ifdef cFPGA
    else if (D->isFPGA())
    {
       fpga_relu_soft_d(D, I, PD, FPGASOFTM);
    }
#endif

}
