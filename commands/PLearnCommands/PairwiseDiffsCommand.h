// -*- C++ -*-

// PairwiseDiffsCommand.h
//
// Copyright (C) 2005 Nicolas Chapados 
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

// Authors: Nicolas Chapados

/*! \file PairwiseDiffsCommand.h */


#ifndef PairwiseDiffsCommand_INC
#define PairwiseDiffsCommand_INC

#include <commands/PLearnCommands/PLearnCommand.h>
#include <commands/PLearnCommands/PLearnCommandRegistry.h>

namespace PLearn {

/**
 *  This command computes a set of statistics (user-specified) on the
 *  pairwise differences between a given column of a list of matrices.  The
 *  typical usage is to give a list of test cost matrices (e.g. such as the
 *  'test_costs.pmat' files generated by PTester), and specify the cost of
 *  interest (column name) in the matrix.  For all of the matrices taken
 *  pairwise, the command will accumulate the DIFFERENCE of the specified
 *  columns in a stats collector, and then will output a set of
 *  user-specified statistics.  The default statistics are 'E' and 'PZ2t'
 *  (see the supported statistics in StatsCollector), meaning that the mean
 *  difference is computed, and the p-value that this difference is zero.
 */
class PairwiseDiffsCommand: public PLearnCommand
{
public:
    PairwiseDiffsCommand();                    
    virtual void run(const std::vector<std::string>& args);

protected:
    static PLearnCommandRegistry reg_;
};

  
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
