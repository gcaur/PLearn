// -*- C++ -*-

// VPLPreprocessedLearner.cc
//
// Copyright (C) 2005 Pascal Vincent 
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
 * $Id$ 
 ******************************************************* */

// Authors: Pascal Vincent

/*! \file VPLPreprocessedLearner.cc */


#include "VPLPreprocessedLearner.h"
#include <plearn/vmat/ProcessingVMatrix.h>
#include <plearn/base/tostring.h>

namespace PLearn {
using namespace std;

VPLPreprocessedLearner::VPLPreprocessedLearner() 
    : newtargetsize(-1),
      newweightsize(-1)
{
}

PLEARN_IMPLEMENT_OBJECT(
    VPLPreprocessedLearner,
    "Learner whose training-set, inputs and outputs can be pre/post-processed by VPL code",
    "See VMatLanguage for the definition of the allowed VPL syntax."
    );

void VPLPreprocessedLearner::declareOptions(OptionList& ol)
{
    // ### Declare all of this object's options here
    // ### For the "flags" of each option, you should typically specify  
    // ### one of OptionBase::buildoption, OptionBase::learntoption or 
    // ### OptionBase::tuningoption. Another possible flag to be combined with
    // ### is OptionBase::nosave

    // ### ex:
    // declareOption(ol, "myoption", &VPLPreprocessedLearner::myoption, OptionBase::buildoption,
    //               "Help text describing this option");
    // ...

    declareOption(ol, "learner", &VPLPreprocessedLearner::learner_,
                  OptionBase::buildoption,
                  "The embedded learner");

    declareOption(ol, "trainset_preproc", &VPLPreprocessedLearner::trainset_preproc, OptionBase::buildoption,
                  "Program string in VPL language to be applied to each row of the initial\n"
                  "training set to generate the new preprocessed training set.\n"
                  "This should generate preprocessed input and target (and weight if any).\n"
                  "Note that names must be given to the generated values with :fieldname VPL syntax.\n"
                  "An empty string means NO PREPROCESSING. (initial training set is used as is)");
    declareOption(ol, "newtargetsize", &VPLPreprocessedLearner::newtargetsize, OptionBase::buildoption,
                  "Size of target produced by trainset_preproc (target must follow preprocessed input).");
    declareOption(ol, "newweightsize", &VPLPreprocessedLearner::newweightsize, OptionBase::buildoption,
                  "0 or 1, depending on whether trainset_preproc generates a weight or not after the target.");

    declareOption(ol, "input_preproc", &VPLPreprocessedLearner::input_preproc, OptionBase::buildoption,
                  "Program string in VPL language to be applied to a new input.\n"
                  "This must produce exactly the same thing as the preprocessed.\n"
                  "input part produced by trainset_preproc\n"
                  "Note that names must be given to the generated values with :fieldname VPL syntax.\n");
  
    declareOption(ol, "output_postproc", &VPLPreprocessedLearner::output_postproc, OptionBase::buildoption,
                  "Program string in VPL language to optain postprocessed output\n"
                  "from the underlying learner's output.\n"
                  "Note that names must be given to the generated values with :fieldname VPL syntax.\n"
                  "An empty string means NO OUTPUT POSTPROCESSING.\n\n"
                  "Additional Note: when this snippet of VPL starts executing, the\n"
                  "ORIGINAL (not preprocessed) input row is stored in VPL memory, starting\n"
                  "from memory location 0 until inputsize-1 (inclusive).  You can access these\n"
                  "memory locations using VPL instructions of the form:\n"
                  "\n"
                  "    <memory_location> memget\n"
                  "\n"
                  "This lets the output postprocessor use some information from the input\n"
                  "vector to construct the output (e.g. put back some data that was hidden\n"
                  "from the embedded learner.)");
    declareOption(ol, "costs_postproc", &VPLPreprocessedLearner::costs_postproc, OptionBase::buildoption,
                  "Program string in VPL language to optain postprocessed test costs\n"
                  "from the underlying learner's test costs.\n"
                  "Note that names must be given to the generated values with :fieldname VPL syntax.\n"
                  "An empty string means NO COSTS POSTPROCESSING.\n"
                  "Note that the *train* costs are those of the underlying learner. No postproc is applied to those.");

    declareOption(ol, "row_prg", &VPLPreprocessedLearner::row_prg, OptionBase::learntoption,
                  "Compiled trainset_preproc program");
    declareOption(ol, "input_prg", &VPLPreprocessedLearner::input_prg, OptionBase::learntoption,
                  "Compiled input_preproc program");
    declareOption(ol, "output_prg", &VPLPreprocessedLearner::output_prg, OptionBase::learntoption,
                  "Compiled output_postproc program");
    declareOption(ol, "costs_prg", &VPLPreprocessedLearner::costs_prg, OptionBase::learntoption,
                  "Compiled costs_postproc program");

    declareOption(ol, "row_prg_fieldnames", &VPLPreprocessedLearner::row_prg_fieldnames, OptionBase::learntoption,
                  "names of fields produced by row_prg");
    declareOption(ol, "input_prg_fieldnames", &VPLPreprocessedLearner::input_prg_fieldnames, OptionBase::learntoption,
                  "names of fields produced by input_prg");
    declareOption(ol, "output_prg_fieldnames", &VPLPreprocessedLearner::output_prg_fieldnames, OptionBase::learntoption,
                  "names of fields produced by output_prg");
    declareOption(ol, "costs_prg_fieldnames", &VPLPreprocessedLearner::costs_prg_fieldnames, OptionBase::learntoption,
                  "names of fields produced by costs_prg");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

void VPLPreprocessedLearner::build_()
{
}

// ### Nothing to add here, simply calls build_
void VPLPreprocessedLearner::build()
{
    inherited::build();
    build_();
}


void VPLPreprocessedLearner::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    // ### Call deepCopyField on all "pointer-like" fields 
    // ### that you wish to be deepCopied rather than 
    // ### shallow-copied.

    deepCopyField(learner_, copies);    

    // deepCopyField(row_prg, copies);
    // deepCopyField(input_prg, copies);
    // deepCopyField(output_prg, copies);
    // deepCopyField(costs_prg, copies);
    row_prg.makeDeepCopyFromShallowCopy(copies);
    input_prg.makeDeepCopyFromShallowCopy(copies);
    output_prg.makeDeepCopyFromShallowCopy(copies);
    costs_prg.makeDeepCopyFromShallowCopy(copies);
 
    deepCopyField(row_prg_fieldnames, copies); 
    deepCopyField(input_prg_fieldnames, copies);
    deepCopyField(output_prg_fieldnames, copies);
    deepCopyField(costs_prg_fieldnames, copies);
    deepCopyField(row, copies);
    deepCopyField(processed_row, copies);
    deepCopyField(processed_input, copies);
    deepCopyField(pre_output, copies);
    deepCopyField(pre_costs, copies);
}

void VPLPreprocessedLearner::setValidationSet(VMat validset)
{
    assert( learner_ );
    inherited::setValidationSet(validset);
    learner_->setValidationSet(validset);
}

void VPLPreprocessedLearner::setTrainStatsCollector(PP<VecStatsCollector> statscol)
{
    assert( learner_ );
    inherited::setTrainStatsCollector(statscol);
    learner_->setTrainStatsCollector(statscol);
}

int VPLPreprocessedLearner::outputsize() const
{
    if(output_prg)
        return output_prg_fieldnames.length();
    else
    {
        assert( learner_ );
        return learner_->outputsize();
    }
}

void VPLPreprocessedLearner::setExperimentDirectory(const PPath& the_expdir)
{
    assert( learner_ );
    inherited::setExperimentDirectory(the_expdir);
    learner_->setExperimentDirectory(the_expdir);
}

void VPLPreprocessedLearner::forget()
{
    assert( learner_);
    learner_->forget();
    stage = 0;
}
    
void VPLPreprocessedLearner::train()
{
    assert( learner_ );
    learner_->train();
    stage = learner_->stage;
}

void VPLPreprocessedLearner::setTrainingSet(VMat training_set, bool call_forget)
{
    assert( learner_ );

    if(trainset_preproc!="")
    {
        row_prg.setSource(training_set);
        row_prg.compileString(trainset_preproc, row_prg_fieldnames);
        int newinputsize = row_prg_fieldnames.length()-(newtargetsize+newweightsize);
        VMat processed_trainset = new ProcessingVMatrix(training_set, trainset_preproc);
        processed_trainset->defineSizes(newinputsize,newtargetsize,newweightsize);
        learner_->setTrainingSet(processed_trainset, false);
    }
    else
    {
        row_prg.clear();
        row_prg_fieldnames.resize(0);
        learner_->setTrainingSet(training_set, false);
    }
    bool training_set_has_changed = !train_set || !(train_set->looksTheSameAs(training_set));
    if (call_forget && !training_set_has_changed)
        // In this case, learner_->build() will not have been called, which may
        // cause trouble if it updates data from the training set.
        learner_->build();
    inherited::setTrainingSet(training_set, call_forget); // will call forget if needed

    if(input_preproc!="")
    {
        int insize = training_set->inputsize();
        TVec<string> infieldnames = training_set->fieldNames().subVec(0,insize);
        input_prg.setSourceFieldNames(infieldnames);
        input_prg.compileString(input_preproc, input_prg_fieldnames);
    }
    else
    {
        input_prg.clear();
        input_prg_fieldnames.resize(0);
    }

    if(output_postproc!="")
    {
        int outsize = learner_->outputsize();
        TVec<string> outfieldnames(outsize);
        for(int k=0; k<outsize; k++)
            outfieldnames[k] = "output"+tostring2(k);
        output_prg.setSourceFieldNames(outfieldnames);
        output_prg.compileString(output_postproc, output_prg_fieldnames);
    }
    else
    {
        output_prg.clear();
        output_prg_fieldnames.resize(0);
    }

    if(costs_postproc!="")
    {
        costs_prg.setSourceFieldNames(learner_->getTestCostNames());
        costs_prg.compileString(costs_postproc, costs_prg_fieldnames);
    }
    else
    {
        costs_prg.clear();
        costs_prg_fieldnames.resize(0);
    }
}


void VPLPreprocessedLearner::computeOutput(const Vec& input, Vec& output) const
{
    assert( learner_ );
    output.resize(outputsize());
    Vec newinput = input;
    if(input_prg)
    {
        processed_input.resize(input_prg_fieldnames.length());
        input_prg.run(input, processed_input);
        newinput = processed_input;
    }

    if(output_prg)
    {
        learner_->computeOutput(newinput, pre_output);
        output_prg.setMemory(input);           // Put original input vector
        // as context for output postproc
        output_prg.run(pre_output, output);
    }
    else
        learner_->computeOutput(newinput, output);
    
}    

void VPLPreprocessedLearner::computeOutputAndCosts(const Vec& input, const Vec& target, 
                                                   Vec& output, Vec& costs) const
{ 
    output.resize(outputsize());
    costs.resize(nTestCosts());

    assert( learner_ );
    int ilen = input.length();
    int tlen = target.length();
    assert(ilen==inputsize());
    assert(tlen==targetsize());

    if(row_prg)
    {
        row.resize(ilen+tlen);
        row.subVec(0,ilen) << input;
        row.subVec(ilen,tlen) << target;
        if(weightsize()==1)
            row.append(1.0);

        int newrowsize = row_prg_fieldnames.length();
        processed_row.resize(newrowsize);
        row_prg.run(row, processed_row);
        int newinputsize = newrowsize-(newtargetsize+newweightsize);
        Vec processed_input = processed_row.subVec(0,newinputsize);
        Vec processed_target = processed_row.subVec(newinputsize,newtargetsize);
        learner_->computeOutputAndCosts(processed_input, processed_target, pre_output, pre_costs);
    }
    else
        learner_->computeOutputAndCosts(input, target, pre_output, pre_costs);

    if(output_prg) {
        output_prg.setMemory(input);           // Put original input vector
        // as context for output postproc
        output_prg.run(pre_output, output);
    }
    else
        output << pre_output;

    if(costs_prg)
        costs_prg.run(pre_costs, costs);
    else
        costs << pre_costs;
}

void VPLPreprocessedLearner::computeCostsFromOutputs(const Vec& input, const Vec& output, 
                                                     const Vec& target, Vec& costs) const
{ 
    Vec nonconst_output = output; // to make the constipated compiler happy
    computeOutputAndCosts(input, target, nonconst_output, costs); 
}

bool VPLPreprocessedLearner::computeConfidenceFromOutput(
    const Vec& input, const Vec& output,
    real probability, TVec< pair<real,real> >& intervals) const
{
    int d = outputsize();
    if(d!=output.length())
        PLERROR("In VPLPreprocessedLearner::computeConfidenceFromOutput, length of passed output (%d)"
                "differes from outputsize (%d)!",output.length(),d);

    assert( learner_ );
    Vec newinput = input;
    if(input_prg)
    {
        processed_input.resize(input_prg_fieldnames.length());
        input_prg.run(input, processed_input);
        newinput = processed_input;
    }

    bool status = false;
    if(!output_prg) // output is already the output of the underlying learner
        status = learner_->computeConfidenceFromOutput(newinput, output, probability, intervals);
    else // must recompute the output of underlying learner, and post-process returned intervals
    {
        learner_->computeOutput(newinput, pre_output);
        TVec< pair<real,real> > pre_intervals;
        status = learner_->computeConfidenceFromOutput(newinput, pre_output, probability, pre_intervals);
        if(!status) // no confidence computation available
        {
            intervals.resize(d);
            for(int k=0; k<d; k++)
                intervals[k] = pair<real,real>(MISSING_VALUE,MISSING_VALUE);
        }
        else // postprocess low and high vectors
        {
            int ud = learner_->outputsize(); // dimension of underlying learner's output
            // first build low and high vectors
            Vec low(ud);
            Vec high(ud);
            for(int k=0; k<ud; k++)
            {
                pair<real,real> p = pre_intervals[k];
                low[k] = p.first;
                high[k] = p.second;
            }
            Vec post_low(d); // postprocesed low
            Vec post_high(d); // postprocessed high

            // Put original input vector as context for output postproc
            output_prg.setMemory(input);
            output_prg.run(low, post_low);

            // Put original input vector as context for output postproc
            output_prg.setMemory(input);
            output_prg.run(high, post_high);

            // Now copy post_low and post_high to intervals
            intervals.resize(d);
            for(int k=0; k<d; k++)
                intervals[k] = pair<real,real>(post_low[k],post_high[k]);
        }
    }
    return status;
}

TVec<string> VPLPreprocessedLearner::getOutputNames() const
{
    if(output_prg)
        return output_prg_fieldnames;
    else
        return learner_->getOutputNames();
}


TVec<string> VPLPreprocessedLearner::getTestCostNames() const
{
    if(costs_prg)
        return costs_prg_fieldnames;
    else
        return learner_->getTestCostNames();
}

TVec<string> VPLPreprocessedLearner::getTrainCostNames() const
{
    assert( learner_ );
    return learner_->getTrainCostNames();
}

void VPLPreprocessedLearner::resetInternalState()
{
    assert( learner_ );
    learner_->resetInternalState();
}

bool VPLPreprocessedLearner::isStatefulLearner() const
{
    assert( learner_ );
    return learner_->isStatefulLearner();
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
