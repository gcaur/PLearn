// -*- C++ -*-

// GradientCorrector.cc
//
// Copyright (C) 2007 Yoshua Bengio
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

// Authors: Yoshua Bengio

/*! \file GradientCorrector.cc */


#include "GradientCorrector.h"
#include <plearn/math/TMat_maths.h>
#include <plearn/math/plapack.h>

namespace PLearn {
using namespace std;

// bool save_G=false;

PLEARN_IMPLEMENT_ABSTRACT_OBJECT(
    GradientCorrector,
    "Virtual class used for correcting a sequence of n-dimensional gradients.\n",
    "The algorithm used for converting a sequence of n-dimensional gradients g_t\n"
    "into covariance-corrected update directions v_t depends on the subclass.\n"
    "The main method is the one that does the conversion:\n"
    "  operator(int t, Vec g, Vec v): (reads g and writes v)\n"
    "There is also an init() method to start a new sequence.\n"
    );

GradientCorrector::GradientCorrector()
    /* ### Initialize all fields to their default value */
    : n_dim(-1),
      verbosity(0)
{
}

// ### Nothing to add here, simply calls build_
void GradientCorrector::build()
{
    inherited::build();
    build_();
}

void GradientCorrector::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);
}

void GradientCorrector::declareOptions(OptionList& ol)
{
    // ### Declare all of this object's options here.
    // ### For the "flags" of each option, you should typically specify
    // ### one of OptionBase::buildoption, OptionBase::learntoption or
    // ### OptionBase::tuningoption. If you don't provide one of these three,
    // ### this option will be ignored when loading values from a script.
    // ### You can also combine flags, for example with OptionBase::nosave:
    // ### (OptionBase::buildoption | OptionBase::nosave)

    declareOption(ol, "n_dim", &GradientCorrector::n_dim,
                  OptionBase::learntoption,
                  "Number of dimensions of the gradient vectors\n");
    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

void GradientCorrector::build_()
{
    init();
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
