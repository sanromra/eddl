/*
* EDDL Library - European Distributed Deep Learning Library.
* Version: 0.6
* copyright (c) 2020, Universidad Politécnica de Valencia (UPV), PRHLT Research Centre
* Date: April 2020
* Author: PRHLT Research Centre, UPV, (rparedes@prhlt.upv.es), (jon@prhlt.upv.es)
* All rights reserved
*/


#ifndef EDDL_GPU_TENSOR_H
#define EDDL_GPU_TENSOR_H


#include <cuda.h>
#include <curand.h>
#include <cuda_runtime_api.h>
#include <cublas_v2.h>

//
//#include <cstdio>
//#include <cuda.h>
//#include <cuda_runtime_api.h>
//#include <cublas_v2.h>
//
//#include "eddl/hardware/gpu/gpu_tensor.h"
//#include "eddl/hardware/gpu/tensor_kernels.h"



void check_cublas(cublasStatus_t status, const char *f);

void check_curand(curandStatus_t status, const char *f);

void check_cuda(cudaError_t err,const char *msg);
void gpu_set_device(int device);
void gpu_init(int device);

float* gpu_create_tensor(int dev,int size);
void gpu_delete_tensor(int dev,float* p);
void gpu_delete_tensor_int(int dev, int* p);

int gpu_devices();

#endif
