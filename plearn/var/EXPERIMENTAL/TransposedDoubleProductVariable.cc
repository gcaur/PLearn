// -*- C++ -*-

// TransposedDoubleProductVariable.cc
//
// Copyright (C) 2007 Simon Lemieux, Pascal Vincent
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//  1. Redistributions of source code must retain the above copyright
//     notice, this list of conditions and the following disclaimer.
//
//  2. Redistributions in binary form must reproduce the above copyright
//     notice, this list of conditions and the following disclaimer in the
//     documentation and/or other materials provided with the distribution.
//
//  3. The name of the authors may not be used to endorse or promote
//     products derived from this software without specific prior written
//     permission.
//
// THIS SOFTWARE IS PROVIDED BY THE AUTHORS ``AS IS'' AND ANY EXPRESS OR
// IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
// OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN
// NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
// TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// This file is part of the PLearn library. For more information on the PLearn
// library, go to the PLearn Web site at www.plearn.org

// Authors: Simon Lemieux, Pascal Vincent

/*! \file TransposedDoubleProductVariable.cc */


#include "TransposedDoubleProductVariable.h"

namespace PLearn {
using namespace std;

/** TransposedDoubleProductVariable **/

PLEARN_IMPLEMENT_OBJECT(
    TransposedDoubleProductVariable,
    "Let H, W and M be the inputs and nw the length of W. Then output(n,k) = sum_i{ sum_j { W(i,k)*M(j,k)*H(n,i+j*nw) } }",
    "MULTI LINE\nHELP FOR USERS"
    );

TransposedDoubleProductVariable::TransposedDoubleProductVariable()
{}

TransposedDoubleProductVariable::TransposedDoubleProductVariable(Var h, Var w, Var m)
    : inherited(h & w & m, h.length(), w.width())
{
    build_();
}


void TransposedDoubleProductVariable::recomputeSize(int& l, int& w) const
{    
        if (varray.size() > 0) {
            // l = varH()->length(); // the computed length of this Var
            //w = varW()->width(); // the computed width
            l = varray[0].length();
            w = varray[1].width();
        } else
            l = w = 0;
}

// ### computes value from varray values
void TransposedDoubleProductVariable::fprop()
{
    Mat w = varW()->matValue,
        m = varM()->matValue,
        h = varH()->matValue;
    
    int nw = w.length(),
        nm = m.length(),
        nx = h.length(),
        d = w.width();

    for(int n=0; n<nx; n++)
        for(int k=0; k<d; k++)
        {
            matValue(n,k)=0.;
            for(int i=0; i<nw; i++)
                for(int j=0; j<nm; j++)
                    matValue(n,k) += w(i,k)*m(j,k)*h(n,i+j*nw);
        }
}

// ### computes varray gradients from gradient
void TransposedDoubleProductVariable::bprop()
{
    Mat w = varW()->matValue,
        m = varM()->matValue,
        h = varH()->matValue,
        w_grad = varW()->matGradient,
        m_grad = varM()->matGradient,
        h_grad = varH()->matGradient,
        x_grad = matGradient;
    
    int nw = w.length(),
        nm = m.length(),
        nx = h.length(),
        d = w.width();

    for(int n=0; n<nx; n++)
        for(int k=0; k<d; k++)
            for(int i=0; i<nw; i++)
                for(int j=0; j<nm; j++)
                {
                    w_grad(i,k) += x_grad(n,k)*m(j,k)*h(n,i+j*nw);
                    m_grad(j,k) += x_grad(n,k)*w(i,k)*h(n,i+j*nw);
                    h_grad(n,i+j*nw) += x_grad(n,k)*w(i,k)*m(j,k);
                }
}

// ### You can implement these methods:
// void TransposedDoubleProductVariable::bbprop() {}
// void TransposedDoubleProductVariable::symbolicBprop() {}
// void TransposedDoubleProductVariable::rfprop() {}


// ### Nothing to add here, simply calls build_
void TransposedDoubleProductVariable::build()
{
    inherited::build();
    build_();
}

void TransposedDoubleProductVariable::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    // ### Call deepCopyField on all "pointer-like" fields
    // ### that you wish to be deepCopied rather than
    // ### shallow-copied.
    // ### ex:
    // deepCopyField(trainvec, copies);

    // ### If you want to deepCopy a Var field:
    // varDeepCopyField(somevariable, copies);
}

void TransposedDoubleProductVariable::declareOptions(OptionList& ol)
{
    // ### Declare all of this object's options here.
    // ### For the "flags" of each option, you should typically specify
    // ### one of OptionBase::buildoption, OptionBase::learntoption or
    // ### OptionBase::tuningoption. If you don't provide one of these three,
    // ### this option will be ignored when loading values from a script.
    // ### You can also combine flags, for example with OptionBase::nosave:
    // ### (OptionBase::buildoption | OptionBase::nosave)

    // ### ex:
    // declareOption(ol, "myoption", &TransposedDoubleProductVariable::myoption,
    //               OptionBase::buildoption,
    //               "Help text describing this option");
    // ...

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

void TransposedDoubleProductVariable::build_()
{
    // ### This method should do the real building of the object,
    // ### according to set 'options', in *any* situation.
    // ### Typical situations include:
    // ###  - Initial building of an object from a few user-specified options
    // ###  - Building of a "reloaded" object: i.e. from the complete set of
    // ###    all serialised options.
    // ###  - Updating or "re-building" of an object after a few "tuning"
    // ###    options have been modified.
    // ### You should assume that the parent class' build_() has already been
    // ### called.

    if (varH().width() != varW().length()*varM().length())
        PLERROR("The width of matrix H is incompatible with the lengths of matrix W and M in TranposedDoubleProductVariable");
    if (varM().width() != varW().width())
        PLERROR("Matrix W and M must have the same width in TranposedDoubleProduct");


}


} // end of namespace PLearn


/*
  Local Variables:
  mode:c++
  c-basic-offset:4
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0))
  indent-tabs-mode:nil
  fill-column:79
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=79 :
