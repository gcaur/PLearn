// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 1998 Pascal Vincent
// Copyright (C) 1999-2002 Pascal Vincent, Yoshua Bengio, Rejean Ducharme and University of Montreal
// Copyright (C) 2001-2002 Nicolas Chapados, Ichiro Takeuchi, Jean-Sebastien Senecal
// Copyright (C) 2002 Xiangdong Wang, Christian Dorion

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
   * $Id: SigmoidVariable.cc,v 1.4 2003/12/16 17:44:52 plearner Exp $
   * This file is part of the PLearn library.
   ******************************************************* */

#include "SigmoidVariable.h"
#include "Var_operators.h"

namespace PLearn <%
using namespace std;


/** SigmoidVariable **/

SigmoidVariable::SigmoidVariable(Variable* input) 
  :UnaryVariable(input, input->length(), input->width()) {}


PLEARN_IMPLEMENT_OBJECT(SigmoidVariable, "ONE LINE DESCR", "NO HELP");

void SigmoidVariable::recomputeSize(int& l, int& w) const
{ l=input->length(); w=input->width(); }








void SigmoidVariable::fprop()
{
  int l = nelems();
  real* valueptr = valuedata;
  real* inputvalueptr = input->valuedata;
  for(int i=0; i<l; i++)
    *valueptr++ = sigmoid(*inputvalueptr++);
}


void SigmoidVariable::bprop()
{
  int l = nelems();
  real* inputgradientptr = input->gradientdata;
  real* gradientptr = gradientdata;
  real* valueptr = valuedata;
  for(int i=0; i<l; i++)
    {
      real val = *valueptr++;
      *inputgradientptr++ += *gradientptr++ * val*(1.0-val);
    }
}


void SigmoidVariable::bbprop()
{
  if (input->diaghessian.length()==0)
    input->resizeDiagHessian();
  for(int i=0; i<nelems(); i++)
  {
    real yi = valuedata[i];
    real fprime = yi*(1-yi);
    input->gradientdata[i] += gradientdata[i] * fprime * fprime;
  }
}


void SigmoidVariable::symbolicBprop()
{
  Var v(this);
  input->accg(g*v*(1. - v));
}


// R{sigmoid(x)} = f(x)(1-f(x))R(x)
void SigmoidVariable::rfprop()
{
  if (rValue.length()==0) resizeRValue();
  int l = nelems();
  real* inputptr = input->rvaluedata;
  real* inputvalueptr = valuedata;
  real* ptr = rvaluedata;
  for(int i=0; i<l; i++)
  {
    real val = *inputvalueptr++;
    *ptr++ = *inputptr++ * val * (1.0 - val);
  }
}

/*!   a soft version of the ordinary max(x1,x2) mathematical operation
  where the hardness parameter controls how close to an actual max(x1,x2)
  we are (i.e. as hardness --> infty we quickly get max(x1,x2), but
  as hardness --> 0 we get the simple average of x1 and x2.
*/
Var softmax(Var x1, Var x2, Var hardness)
{
  Var w=sigmoid(hardness*(x1-x2));
  return x1*w + x2*(1-w);
}


%> // end of namespace PLearn


