// -*- C++ -*-

// CompareLearner.h
//
// Copyright (C) 2004 Hugo Larochelle 
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
   * $Id: CompareLearner.h,v 1.2 2005/02/08 21:59:56 tihocan Exp $ 
   ******************************************************* */

// Authors: Hugo Larochelle

/*! \file CompareLearner.h */


#ifndef CompareLearner_INC
#define CompareLearner_INC

#include <plearn_learners/generic/PLearner.h>

namespace PLearn {

class CompareLearner: public PLearner
{

private:

  typedef PLearner inherited;
  
protected:

  // *********************
  // * protected options *
  // *********************

  // ### declare protected option fields (such as learnt parameters) here
  // ...

  //! Number of learners to compare
  int n_learners;
  //! Output size of the learners
  int learners_outputsize;
  //! Indexes of the different costs
  TMat<int> costs_indexes;

public:

  // ************************
  // * public build options *
  // ************************

  //! Learners to compare
  TVec< PP<PLearner> > learners;
  //! Names of the learners
  TVec<string> learner_names;
  //! Common costs of the learners
  TVec<string> common_costs;
  
  // ****************
  // * Constructors *
  // ****************

  //! Default constructor.
  CompareLearner();


  // ********************
  // * PLearner methods *
  // ********************

private: 

  //! This does the actual building. 
  void build_();

protected: 
  
  //! Declares this class' options.
  static void declareOptions(OptionList& ol);

public:

  // ************************
  // **** Object methods ****
  // ************************

  //! Simply calls inherited::build() then build_().
  virtual void build();

  //! Transforms a shallow copy into a deep copy.
  virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

  // Declares other standard object methods.
  // If your class is not instantiatable (it has pure virtual methods)
  // you should replace this by PLEARN_DECLARE_ABSTRACT_OBJECT.
  PLEARN_DECLARE_OBJECT(CompareLearner);


  // **************************
  // **** PLearner methods ****
  // **************************

  //! Returns the size of this learner's output, (which typically
  //! may depend on its inputsize(), targetsize() and set options).
  virtual int outputsize() const;

  //! (Re-)initializes the PLearner in its fresh state (that state may depend on the 'seed' option)
  //! And sets 'stage' back to 0 (this is the stage of a fresh learner!).
  virtual void forget();

    
  //! The role of the train method is to bring the learner up to stage==nstages,
  //! updating the train_stats collector with training costs measured on-line in the process.
  virtual void train();


  //! Computes the output from the input.
  virtual void computeOutput(const Vec& input, Vec& output) const;

  //! Computes the costs from already computed output. 
  virtual void computeCostsFromOutputs(const Vec& input, const Vec& output, 
                                       const Vec& target, Vec& costs) const;
                                

  //! Returns the names of the costs computed by computeCostsFromOutpus (and thus the test method).
  virtual TVec<std::string> getTestCostNames() const;

  //! Returns the names of the objective costs that the train method computes and 
  //! for which it updates the VecStatsCollector train_stats.
  virtual TVec<std::string> getTrainCostNames() const;

  virtual void setTrainStatsCollector(PP<VecStatsCollector> statscol);

  virtual void setTrainingSet(VMat training_set, bool call_forget=true);
  
  virtual void setValidationSet(VMat validset);

  virtual void setExperimentDirectory(const PPath& the_expdir);

};
  
  // Declares a few other classes and functions related to this class.
  DECLARE_OBJECT_PTR(CompareLearner);

} // end of namespace PLearn

#endif
