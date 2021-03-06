// -*- C++ -*-

// TruncatedRealFunction.cc
//
// Copyright (C) 2006 Pascal Vincent
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

// Authors: Pascal Vincent

/*! \file TruncatedRealFunction.cc */


#include "TruncatedRealFunction.h"

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    TruncatedRealFunction,
    "An real function that truncates the selected feature between minval and maxval",
    "Also maps missing value to val_for_missing"
    );

// ### Nothing to add here, simply calls build_
void TruncatedRealFunction::build()
{
    inherited::build();
    build_();
}

void TruncatedRealFunction::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);
}

void TruncatedRealFunction::declareOptions(OptionList& ol)
{
    // ### Declare all of this object's options here.
    // ### For the "flags" of each option, you should typically specify
    // ### one of OptionBase::buildoption, OptionBase::learntoption or
    // ### OptionBase::tuningoption. If you don't provide one of these three,
    // ### this option will be ignored when loading values from a script.
    // ### You can also combine flags, for example with OptionBase::nosave:
    // ### (OptionBase::buildoption | OptionBase::nosave)

    declareOption(ol, "minval", &TruncatedRealFunction::minval,
                  OptionBase::buildoption,
                  "If x[which_feature]<=minval, minval is returned.");

    declareOption(ol, "maxval", &TruncatedRealFunction::maxval,
                  OptionBase::buildoption,
                  "If x[which_feature]>=maxval, maxval is returned.");

    declareOption(ol, "val_for_missing", &TruncatedRealFunction::maxval,
                  OptionBase::buildoption,
                  "Value to be returned if x[which_feature] is missing (NaN).");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

real TruncatedRealFunction::evaluateFeature(real x) const
{
    if(is_missing(x))
        return val_for_missing;
    else if(x<=minval)
        return minval;
    else if(x>=maxval)
        return maxval;
    return x;
}

void TruncatedRealFunction::build_()
{}


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
