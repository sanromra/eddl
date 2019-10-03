
/////////////////////////////////////////////////////////////////////////////
// This file is part of EDDLL an European Distributed Deep Learning Library.
// Developed within the DeepHealth project.
// Boosting AI in Europe.
//
// Main authors and developers:
//      Roberto Paredes: rparedes@prhlt.upv.es
//      Joan Ander Gómez: jon@prhlt.upv.es
//
//
// Collaborators:
//      Juan Maroñas: jmaronas@prhlt.upv.es
//
//
// To collaborate please contact rparedes@prhlt.upv.es
//
/////////////////////////////////////////////////////////////////////////////



#ifndef _TENSOR_CUDA_KERNELS_
#define _TENSOR_CUDA_KERNELS_


#include <cuda.h>


// GPU: Activations
__global__ void relu(float *a,float *b,long int size);
__global__ void d_relu(float *d,float *i,float *pd,long int size);
__global__ void softmax(float* E,float* N,float* auxE ,long int sample_ndim, long int n_vals);

// GPU: Losses
__global__ void cent(float* a, float* b, float* c, long int size);

// GPU: Metrics
__global__ void accuracy(float* T, float* N,float* acc,long int cols, long int total_ops, int* MC_err);

// GPU: Conv
__global__ void gpu_addbias_k(float *O, int b, int r,int c,int nk,float *bias);
__global__ void gpu_deltabias_k(float *D, int batch, int r,int c,int nk,float *bias);
__global__ void gpu_im2col_k(float* I, float *ptrI, int b,int irows,int icols, int idepth, float* K, int nk, int kr,int kc, float* O,int orows,int ocols,int sr,int sc,int pad,int col2im);

// GPU: Pool
__global__ void maxpool2d(float* I, int batch,int irows,int icols, int idepth, int kr,int kc, float* O,int orows,int ocols, int odepth, int sr,int sc,int padr, int padc, float* indX, float* indY);
__global__ void maxpool2d_back(float* I, int batch,int irows,int icols, int idepth, int kr,int kc, int sr,int sc,int padr, int padc, float* indX, float* indY, float* D, float* ID);


#endif

