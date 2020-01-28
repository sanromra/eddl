/*
* EDDL Library - European Distributed Deep Learning Library.
* Version: 0.2
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
#include "serialization/onnx/eddl_onnx.h"

using namespace eddl;

//////////////////////////////////
// mnist_mlp.cpp:
// A very basic MLP for mnist
// Using fit for training
//////////////////////////////////


int main(int argc, char **argv) { 
    // Download mnist
    download_mnist();

    // Settings
    int epochs = 1;
    int batch_size = 100;
    int num_classes = 10;

    // Define network
    layer in = Input({784});
    layer l = in;  // Aux var

  	l=Reshape(l,{-1});
    l = ReLu(Dense(l, 1024));
    l = ReLu(Dense(l, 1024));
    l = ReLu(Dense(l, 1024));

    layer out = Activation(Dense(l, num_classes), "softmax");
    model net = Model({in}, {out});

    // Build model
    build(net,
          rmsprop(0.01), // Optimizer
          {"soft_cross_entropy"}, // Losses
          {"categorical_accuracy"}, // Metrics
          //CS_GPU({1}) // one GPU
          CS_CPU(4), // CPU with maximum threads availables
		  true       // Parameter that indicates that the weights of the net must be initialized to random values.
    );

    // View model
    summary(net);

    // Load dataset
    tensor x_train = eddlT::load("trX.bin");
    tensor y_train = eddlT::load("trY.bin");
    tensor x_test = eddlT::load("tsX.bin");
    tensor y_test = eddlT::load("tsY.bin");


    // Preprocessing
    eddlT::div_(x_train, 255.0);
    eddlT::div_(x_test, 255.0);

    // Train model
    fit(net, {x_train}, {y_train}, batch_size, epochs);

    // Evaluate
    evaluate(net, {x_test}, {y_test});

	

	// Export
	string path("example.onnx");
	save_net_to_onnx_file(net, path);
	
	LConv::reset_name_counter(); //Necessary to get matching names
	LDense::reset_name_counter();//Necessary to get matching names
	
	// Import
	Net* imported_net = import_net_from_onnx_file(path);


    // Build model
    build(imported_net,
          rmsprop(0.01), // Optimizer
          {"soft_cross_entropy"}, // Losses
          {"categorical_accuracy"}, // Metrics
          //CS_GPU({1}) // one GPU
          CS_CPU(4), // CPU with maximum threads availables
		  false      // Parameter that indicates that current net weights should remain untouched.
    );

	imported_net->resize(batch_size);

    // Evaluate
    evaluate(imported_net, {x_test}, {y_test});

	return 0;
}


///////////