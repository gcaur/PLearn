// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 1998 Pascal Vincent
// Copyright (C) 1999-2001 Pascal Vincent, Yoshua Bengio, Rejean Ducharme and University of Montreal
// Copyright (C) 2002 Pascal Vincent, Julien Keable, Xavier Saint-Mleux
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


/* *******************************************************      
   * $Id: CrossReferenceVMatrix.cc,v 1.4 2004/04/05 22:50:25 morinf Exp $
   ******************************************************* */

#include "CrossReferenceVMatrix.h"

namespace PLearn {
using namespace std;


/** CrossReferenceVMatrix **/

PLEARN_IMPLEMENT_OBJECT(CrossReferenceVMatrix, "ONE LINE DESC", "ONE LINE HELP");

CrossReferenceVMatrix::CrossReferenceVMatrix()
  : col1(0)
{
}

CrossReferenceVMatrix::CrossReferenceVMatrix(VMat v1, int c1, VMat v2)
 : inherited(v1.length(), v1.width()+v2.width()-1), vm1(v1), col1(c1), vm2(v2)
{
    //fieldinfos = v1->getFieldInfos();
    // fieldinfos &= v2->getFieldInfos();
  build();
}


void
CrossReferenceVMatrix::declareOptions(OptionList &ol)
{
    declareOption(ol, "vm1", &CrossReferenceVMatrix::vm1, OptionBase::buildoption, "");
    declareOption(ol, "vm2", &CrossReferenceVMatrix::vm2, OptionBase::buildoption, "");
    declareOption(ol, "col1", &CrossReferenceVMatrix::col1, OptionBase::buildoption, "");
    inherited::declareOptions(ol);
}

void
CrossReferenceVMatrix::build()
{
    inherited::build();
    build_();
}

void
CrossReferenceVMatrix::build_()
{
    if (vm1 && vm2) {
        fieldinfos = vm1->getFieldInfos();
        fieldinfos &= vm2->getFieldInfos();
    }
}

void CrossReferenceVMatrix::getRow(int i, Vec samplevec) const
{
#ifdef BOUNDCHECK
  if (i<0 || i>=length() || samplevec.length()!=width())
    PLERROR("In CrossReferenceVMatrix::getRow OUT OF BOUNDS");
#endif

  Vec v1(vm1.width());
  Vec v2(vm2.width());
  vm1->getRow(i, v1);
  int index = (int)v1[col1];
  vm2->getRow(index, v2);

  for (int j=0; j<col1; j++) samplevec[j] = v1[j];
  for (int j=col1+1; j<v1.length(); j++) samplevec[j-1] = v1[j];
  for (int j=0; j<v2.length(); j++) samplevec[j+v1.length()-1] = v2[j];
}

real CrossReferenceVMatrix::get(int i, int j) const
{
#ifdef BOUNDCHECK
  if(i<0 || i>=length() || j<0 || j>=width())
    PLERROR("In CrossReferenceVMatrix::get OUT OF BOUNDS");
#endif

  if (j < col1)
    return vm1->get(i,j);
  else if (j < vm1.width()-1)
    return vm1->get(i,j+1);
  else {
    int ii = (int)vm1->get(i,col1);
    int jj = j - vm1.width() + 1;
    return vm2->get(ii,jj);
  }
}

} // end of namespcae PLearn
