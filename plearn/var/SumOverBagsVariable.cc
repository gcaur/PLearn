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
   * $Id: SumOverBagsVariable.cc,v 1.4 2004/02/20 14:50:23 yoshua Exp $
   * This file is part of the PLearn library.
   ******************************************************* */

#include "SumOverBagsVariable.h"
#include "PLMPI.h"
#include "DisplayUtils.h"

namespace PLearn <%
using namespace std;



/** SumOverBagsVariable **/

PLEARN_IMPLEMENT_OBJECT(SumOverBagsVariable, "Variable that sums the value of a Func each time evaluated on a subsequence of a VMat\n", 
                        "returns\n"
                        "   Sum_{bags in vmat} f(inputs and targets in bag)\n"
                        "By convention a bag is a sequence of rows of the vmat in which the last column of the target\n"
                        "indicates whether the row is the first one (and/or) the last one, with its two least significant bits:\n"
                        "   last_column_of_target == 1 ==> first row\n"
                        "   last_column_of_target == 2 ==> last row\n"
                        "   last_column_of_target == 0 ==> intermediate row\n"
                        "   last_column_of_target == 1+2==3 ==> single-row bag (both first and last).\n"
                        "The option n_samples controls how many terms in the sum are considered at a time:\n"
                        "   n_samples <= 0: sum over the whole vmat (e.g. for batch gradient computation)\n"
                        "   n_samples = 1: sum over a single bag at a time (e.g. for stochastic gradient)\n"
                        "                  where each fprop or fbprop advances to the next bag\n"
                        "   otherwise: sum over n_samples bags at a time (e.g. for min-batch training)\n"
                        "The last column of the target is not given in the call to f, but a bag_size input is provided instead.\n"
                        "The inputs to f are: (matrix of bag inputs, the bag size, the bag target, the bag weight).\n"
                        );


SumOverBagsVariable::SumOverBagsVariable(VMat the_vmat, Func the_f, int max_bagsize, int nsamples)
  :NaryVariable(nonInputParentsOfPath(the_f->inputs,the_f->outputs), 
                the_f->outputs[0]->length(), 
                the_f->outputs[0]->width()),
   vmat(the_vmat), f(the_f), max_bag_size(max_bagsize), n_samples(nsamples),
   curpos(0), bag_size(0)
{
  build();
}

void SumOverBagsVariable::build()
{
  inherited::build();
  build_();
}

void SumOverBagsVariable::build_()
{
  if (vmat)
  {
    if (vmat->weightsize()!=0 && vmat->weightsize()!=1)
      PLERROR("SumOverBagsVariable expected vmat->weightsize to be 0 or 1");
    
    input_values.resize(max_bag_size,vmat->inputsize());
    output_value.resize(f->outputs[0]->nelems());
    output_av = Array<Vec>(output_av);
    gradient_av = Array<Vec>(gradient);
    f->inputs.setDontBpropHere(true);

    bag_size_vec.resize(1);
    bag_target_and_bag_signal.resize(vmat->targetsize());
    bag_target = bag_target_and_bag_signal.subVec(0,vmat->targetsize()-1);
    bag_signal = bag_target_and_bag_signal.subVec(vmat->targetsize()-1,1);
    bag_weight.resize(vmat->weightsize());
    f_inputs.resize(4);
    f_inputs[0] = input_values.toVec();
    f_inputs[1] = bag_size_vec;
    f_inputs[2] = bag_target;
    f_inputs[3] = bag_weight;
    unused_gradients.resize(4);
    for (int i=0;i<4;i++) unused_gradients[i] = f_inputs[i].copy();
  }
}

void SumOverBagsVariable::declareOptions(OptionList& ol)
{
  declareOption(ol, "f", &SumOverBagsVariable::f, OptionBase::buildoption, 
                "    Func that is applied on each bag, whose input is the following array of Vars:\n"
                "    (matrix of bag inputs, the bag size, the bag target, the bag weight).\n");

  declareOption(ol, "vmat", &SumOverBagsVariable::vmat, OptionBase::buildoption, 
                "    VMatrix that contains the data, with multiple consecutive rows forming one bag.\n"
                "    The last column of the target indicates the beginning and end of each bag, as follows:\n"
                "   last_column_of_target == 1 ==> first row\n"
                "   last_column_of_target == 2 ==> last row\n"
                "   last_column_of_target == 0 ==> intermediate row\n"
                "   last_column_of_target == 1+2==3 ==> single-row bag (both first and last).\n");

  declareOption(ol, "max_bag_size", &SumOverBagsVariable::max_bag_size, OptionBase::buildoption, 
                "    maximum number of examples in a bag (more than that in vmat will trigger a run-time error).\n");

  declareOption(ol, "n_samples", &SumOverBagsVariable::n_samples, OptionBase::buildoption, 
                "    number of bags to iterate over (1 for online gradient, <=0 for batch).");

  inherited::declareOptions(ol);
}

void SumOverBagsVariable::recomputeSize(int& l, int& w) const
{ l=f->outputs[0]->length(); w=f->outputs[0]->width(); }


void SumOverBagsVariable::makeDeepCopyFromShallowCopy(map<const void*, void*>& copies)
{
  NaryVariable::makeDeepCopyFromShallowCopy(copies);
  deepCopyField(vmat, copies);
  deepCopyField(f, copies);
  deepCopyField(output_value, copies);
  deepCopyField(input_values, copies);
  deepCopyField(bag_size_vec, copies);
  deepCopyField(bag_target_and_bag_signal, copies);
  deepCopyField(bag_target, copies);
  deepCopyField(bag_signal, copies);
  deepCopyField(bag_weight, copies);
  deepCopyField(f_inputs, copies);
  deepCopyField(unused_gradients, copies);
  deepCopyField(output_av, copies);
  deepCopyField(gradient_av, copies);
}


void SumOverBagsVariable::fpropOneBag(bool do_bprop)
{
  static real dummy_weight=0;
  bool reached_end_of_bag=false;
  input_values.resize(max_bag_size,input_values.width());
  for (bag_size=0;!reached_end_of_bag;bag_size++)
  {
    if (bag_size>=max_bag_size)
      PLERROR("SumOverBagsVariable: bag size=%d > expected max. bag size(%d)",
              bag_size,max_bag_size);
    Vec input_value = input_values(bag_size);
    if (vmat->weightsize()>0)
      {
        real& weight = bag_weight[0];
        vmat->getExample(curpos,input_value,bag_target_and_bag_signal,weight);
      }
    else
      vmat->getExample(curpos,input_value,bag_target_and_bag_signal,dummy_weight);
    if (bag_size==0 && !(int(bag_signal[0]) & 1))
      PLERROR("SumOverBagsVariable: data synchronization error, first row of bag has wrong bag signal");
    reached_end_of_bag = (int(bag_signal[0]) & 2);
    if(++curpos == vmat->length())
    {
      curpos = 0;
      if (!reached_end_of_bag)
        {
          PLERROR("SumOverBagsVariable: last bag of VMatrix is not complete");
          return;
        }
        break;
    }
  }
  bag_size_vec[0]=bag_size;
  if (do_bprop)
    f->fbprop(f_inputs,output_av,unused_gradients,gradient_av);
  else
    f->fprop(f_inputs,output_av);
  value += output_value;
}

void SumOverBagsVariable::fprop()
{
  value.clear();
  f->recomputeParents();
  if (n_samples==1)
    fpropOneBag();
  else if (n_samples<=0) // one pass through the whole data set
    {
      curpos=0;
      do 
        fpropOneBag();
      while (curpos>0);
    }
  else 
    for (int i=0;i<n_samples;i++)
      fpropOneBag();
}


void SumOverBagsVariable::fbprop()
{
  value.clear();
  f->recomputeParents();
  if (n_samples==1)
    fpropOneBag(true);
  else if (n_samples<=0) // one pass through the whole data set
    {
      curpos=0;
      do 
        fpropOneBag();
      while (curpos>0);
    }
  else 
    for (int i=0;i<n_samples;i++)
      fpropOneBag(true);
}

void SumOverBagsVariable::bprop()
{ 
  fbprop();
}

void SumOverBagsVariable::printInfo(bool print_gradient)
{
  f->fproppath.printInfo(print_gradient);
  cout << info() << " : " << getName() << "(max_bag_size=" << max_bag_size << ", ";
  cout << ", n_samples=" << n_samples << ") = " << value;
  if (print_gradient) cout << " gradient=" << gradient;
  cout << endl; 
}



%> // end of namespace PLearn


