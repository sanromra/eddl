
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
//      Salva Carrión: salcarpo@prhlt.upv.es
//      Mario Parreño: maparla@prhlt.upv.es
//
//
// To collaborate please contact rparedes@prhlt.upv.es
//
/////////////////////////////////////////////////////////////////////////////

#ifndef EDDL_LAYER_OPERATORS_H
#define EDDL_LAYER_OPERATORS_H


#include <string>
#include <stdio.h>

#include "../layer.h"

#define TRMODE 1
#define TSMODE 0

using namespace std;


/////////////////////////////////////////
/////////////////////////////////////////
// Operator layer
class OperatorLayer : public Layer {
public:

    int binary;
    float val;

    OperatorLayer(string name, int dev);

    void addchild(Layer *l) override;

    void addparent(Layer *l) override;

    string plot(int c) override;


};

/// Abs Layer
class LAbs : public OperatorLayer {
public:
    static int total_layers;

    Tensor *mask;

    LAbs(Layer *l, string name, int dev);

    void forward() override;

    void backward() override;

    Layer *share(int c, int bs, vector<Layer *> p) override;

    Layer *clone(int c, int bs, vector<Layer *> p, int todev) override;
};

/// Diff Layer
class LDiff : public OperatorLayer {
public:
    static int total_layers;

    LDiff(Layer *l1, Layer *l2, string name, int dev);
    LDiff(Layer *l, float k, string name, int dev);

    void forward() override;

    void backward() override;

    Layer *share(int c, int bs, vector<Layer *> p) override;

    Layer *clone(int c, int bs, vector<Layer *> p, int todev) override;
};

/// Div Layer
class LDiv : public OperatorLayer {
public:
    static int total_layers;

    LDiv(Layer *l1, Layer *l2, string name, int dev);
    LDiv(Layer *l, float k, string name, int dev);

    void forward() override;

    void backward() override;

    Layer *share(int c, int bs, vector<Layer *> p) override;

    Layer *clone(int c, int bs, vector<Layer *> p, int todev) override;
};

/// Exp Layer
class LExp : public OperatorLayer {
public:
    static int total_layers;

    LExp(Layer *l, string name, int dev);

    void forward() override;

    void backward() override;

    Layer *share(int c, int bs, vector<Layer *> p) override;

    Layer *clone(int c, int bs, vector<Layer *> p, int todev) override;
};

/// Log Layer
class LLog : public OperatorLayer {
public:
    static int total_layers;

    LLog(Layer *l, string name, int dev);

    void forward() override;

    void backward() override;

    Layer *share(int c, int bs, vector<Layer *> p) override;

    Layer *clone(int c, int bs, vector<Layer *> p, int todev) override;
};

/// Log2 Layer
class LLog2 : public OperatorLayer {
public:
    static int total_layers;

    LLog2(Layer *l, string name, int dev);

    void forward() override;

    void backward() override;

    Layer *share(int c, int bs, vector<Layer *> p) override;

    Layer *clone(int c, int bs, vector<Layer *> p, int todev) override;
};

/// Log10 Layer
class LLog10 : public OperatorLayer {
public:
    static int total_layers;

    LLog10(Layer *l, string name, int dev);

    void forward() override;

    void backward() override;

    Layer *share(int c, int bs, vector<Layer *> p) override;

    Layer *clone(int c, int bs, vector<Layer *> p, int todev) override;
};

/// Mean Layer
/*class LMean : public OperatorLayer {
public:
    static int total_layers;
    tshape os;
    vector<int> axis;
    bool keepdims;

    LMean(Layer *l, vector<int> &axis, bool keepdims, string name, int dev);
    LMean(Layer *l, vector<int> axis, bool keepdims, string name, int dev);

    void forward() override;

    void backward() override;

    Layer *share(int c, int bs, vector<Layer *> p) override;

    Layer *clone(int c, int bs, vector<Layer *> p, int todev) override;
};*/

/// Mult Layer
class LMult : public OperatorLayer {
public:
    static int total_layers;

    LMult(Layer *l1, Layer *l2, string name, int dev);
    LMult(Layer *l, float k, string name, int dev);

    void forward() override;

    void backward() override;

    Layer *share(int c, int bs, vector<Layer *> p) override;

    Layer *clone(int c, int bs, vector<Layer *> p, int todev) override;
};

/// Pow Layer
class LPow : public OperatorLayer {
public:
    static int total_layers;

    LPow(Layer *l1, Layer *l2, string name, int dev);
    LPow(Layer *l, float k, string name, int dev);

    void forward() override;

    void backward() override;

    Layer *share(int c, int bs, vector<Layer *> p) override;

    Layer *clone(int c, int bs, vector<Layer *> p, int todev) override;
};

/// Sqrt Layer
class LSqrt : public OperatorLayer {
public:
    static int total_layers;

    LSqrt(Layer *l, string name, int dev);

    void forward() override;

    void backward() override;

    Layer *share(int c, int bs, vector<Layer *> p) override;

    Layer *clone(int c, int bs, vector<Layer *> p, int todev) override;
};

/// Sum Layer
class LSum : public OperatorLayer {
public:
    static int total_layers;

    LSum(Layer *l1, Layer *l2, string name, int dev);
    LSum(Layer *l, float k, string name, int dev);

    void forward() override;

    void backward() override;

    Layer *share(int c, int bs, vector<Layer *> p) override;

    Layer *clone(int c, int bs, vector<Layer *> p, int todev) override;
};

/// Var Layer
/*class LVar : public OperatorLayer {
public:
    static int total_layers;
    tshape os;
    vector<int> axis;
    bool keepdims;
    Tensor *mean;
    int rsize;
    vector<Layer *> layers;

    LVar(Layer *l, vector<int> &axis, bool keepdims, string name, int dev);
    LVar(Layer *l, vector<int> axis, bool keepdims, string name, int dev);

    void forward() override;

    void backward() override;

    Layer *share(int c, int bs, vector<Layer *> p) override;

    Layer *clone(int c, int bs, vector<Layer *> p, int todev) override;
};*/

#endif //EDDL_LAYER_OPERATORS_H
