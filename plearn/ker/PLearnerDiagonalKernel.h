// -*- C++ -*-

// PLearnerDiagonalKernel.h
//
// Copyright (C) 2009 Nicolas Chapados
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

// Authors: Nicolas Chapados

/*! \file PLearnerDiagonalKernel.h */


#ifndef MATERN1ARDKERNEL_INC
#define MATERN1ARDKERNEL_INC

#include <plearn/ker/KroneckerBaseKernel.h>
#include <plearn_learners/generic/PLearner.h>

namespace PLearn {

/**
 *  Diagonal kernel from the output of a PLearner.
 *
 *  The output of this kernel is given by:
 *
 *    k(x,x) = isp_signal_sigma * exp(learner->computeOutput(x))
 *
 *  and is 0 for x != y.
 *
 *  This is useful for representing heteroscedastic noise in Gaussian
 *  Processes, where the log-noise process is the output of another learner
 *  (e.g. another Gaussian Process).
 */
class PLearnerDiagonalKernel : public KroneckerBaseKernel
{
    typedef KroneckerBaseKernel inherited;

public:
    //#####  Public Build Options  ############################################

    /// Learner we are taking output from.
    PP<PLearner> m_learner;
    
    /// Inverse softplus of the global noise variance.  Default value = 0.0.
    mutable real m_isp_signal_sigma;
    
public:
    //#####  Public Member Functions  #########################################

    /// Default constructor
    PLearnerDiagonalKernel();


    //#####  Kernel Member Functions  #########################################

    /// Compute K(x1,x2).
    virtual real evaluate(const Vec& x1, const Vec& x2) const;

    /// Compute the Gram Matrix.
    virtual void computeGramMatrix(Mat K) const;
    
    /// Directly compute the derivative with respect to hyperparameters
    /// (Faster than finite differences...)
    virtual void computeGramMatrixDerivative(Mat& KD, const string& kernel_param,
                                             real epsilon=1e-6) const;
    
    /// Fill k_xi_x with K(x_i, x), for all i from istart to istart + k_xi_x.length() - 1.
    virtual void evaluate_all_i_x(const Vec& x, const Vec& k_xi_x,
                                  real squared_norm_of_x=-1, int istart = 0) const;


    //#####  PLearn::Object Protocol  #########################################

    // Declares other standard object methods.
    PLEARN_DECLARE_OBJECT(PLearnerDiagonalKernel);

    // Simply calls inherited::build() then build_()
    virtual void build();

    /// Transforms a shallow copy into a deep copy
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

protected:
    /// Declares the class options.
    static void declareOptions(OptionList& ol);

    /// Compute derivative w.r.t. isp_signal_sigma for WHOLE MATRIX
    void computeGramMatrixDerivIspSignalSigma(Mat& KD) const;

private:
    /// This does the actual building.
    void build_();

private:
    /// Buffer for evaluation of computeOutput
    mutable Vec m_output_buffer;
};

// Declares a few other classes and functions related to this class
DECLARE_OBJECT_PTR(PLearnerDiagonalKernel);

} // end of namespace PLearn

#endif


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
