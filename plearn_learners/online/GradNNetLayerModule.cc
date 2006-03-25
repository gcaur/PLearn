// -*- C++ -*-

// GradNNetLayerModule.cc
//
// Copyright (C) 2005 Pascal Lamblin 
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
   * $Id: GradNNetLayerModule.cc,v 1.3 2006/01/18 04:04:06 lamblinp Exp $ 
   ******************************************************* */

// Authors: Pascal Lamblin

/*! \file GradNNetLayerModule.cc */


#include "GradNNetLayerModule.h"

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    GradNNetLayerModule,
    "Neural Network layer, using stochastic gradient to update neuron weights",
    "       Output = weights * (1,Input)\n"
    "Weights are updated by online gradient with learning rate possibly decreasing\n"
    "in 1/(1 + n_updates_done_up_to_now * decrease_constant).\n"
    "An L1 and/or L2 regularization penalty can be added to push weights to 0.\n"
    "Weights can be initialized to 0, to a given initial matrix, or randomly\n"
    "from a uniform distribution.\n"
    );

GradNNetLayerModule::GradNNetLayerModule():
    start_learning_rate( .001 ),
    decrease_constant( 0 ),
    init_weights_random_scale( 1. ),
    L1_penalty_factor( 0. ),
    L2_penalty_factor( 0. ),
    step_number( 0 )
    /* ### Initialize all fields to their default value */
{
}

// Applies linear transformation
void GradNNetLayerModule::fprop(const Vec& input, Vec& output) const
{
    int in_size = input.size();

    // size check
    if( in_size != input_size )
    {
        PLERROR("GradNNetLayerModule::fprop: 'input.size()' should be equal\n"
                " to 'input_size' (%i != %i)\n", in_size, input_size);
    }

    Vec bias_input( 1+input_size ); // generalized input (with 1 as first coord)
    bias_input[0] = 1;
    bias_input.subVec( 1, input_size ) << input;

    product( output, weights, bias_input );

}

void GradNNetLayerModule::bpropUpdate(const Vec& input, const Vec& output,
                                      const Vec& output_gradient)
{
    int in_size = input.size();
    int out_size = output.size();
    int og_size = output_gradient.size();

    // size check
    if( in_size != input_size )
    {
        PLERROR("GradNNetLayerModule::bpropUpdate: 'input.size()' should be"
                " equal\n"
                " to 'input_size' (%i != %i)\n", in_size, input_size);
    }
    if( out_size != output_size )
    {
        PLERROR("GradNNetLayerModule::bpropUpdate: 'output.size()' should be"
                " equal\n"
                " to 'output_size' (%i != %i)\n", out_size, output_size);
    }
    if( og_size != output_size )
    {
        PLERROR("GradNNetLayerModule::bpropUpdate: 'output_gradient.size()'"
                " should\n"
                " be equal to 'output_size' (%i != %i)\n",
                og_size, output_size);
    }

    learning_rate = start_learning_rate / ( 1+decrease_constant*step_number);

    Vec bias_input( 1+input_size );
    bias_input[0] = 1;
    bias_input.subVec( 1,input_size ) << input;

    externalProductAcc( weights, -learning_rate*output_gradient, bias_input );

    if (L1_penalty_factor!=0)
    {
        real delta = learning_rate * L1_penalty_factor;
        for (int i=0;i<output_size;i++)
        {
            real* Wi = weights[i]+1; // don't apply penalty on bias
            for (int j=0;j<input_size;j++)
            {
                real Wij =  Wi[j];
                if (Wij>delta)
                    Wi[j] -=delta;
                else if (Wij<-delta)
                    Wi[j] +=delta;
                else 
                    Wi[j]=0;
            }
        }
    }
    if (L2_penalty_factor!=0)
    {
        real delta = learning_rate*L2_penalty_factor;
        if (delta>1)
            PLWARNING("GradNNetLayerModule::bpropUpdate: learning rate = %f is too large!",learning_rate);
        weights.subMatColumns(1,input_size) *= 1 - delta; // no weight decay on the bias
    }

    step_number++;

}


// Simply updates and propagates back gradient
void GradNNetLayerModule::bpropUpdate(const Vec& input, const Vec& output,
                                      Vec& input_gradient,
                                      const Vec& output_gradient)
{
    // compute input_gradient from initial weights
    input_gradient = transposeProduct( weights, output_gradient
                                     ).subVec( 1, input_size );

    // do the update (and size check)
    bpropUpdate( input, output, output_gradient);


}

// Update
void GradNNetLayerModule::bbpropUpdate(const Vec& input, const Vec& output,
                                       const Vec& output_gradient,
                                       const Vec& output_diag_hessian)
{
    PLWARNING("GradNNetLayerModule::bbpropUpdate: You're providing\n"
              "'output_diag_hessian', but it will not be used.\n");

    int odh_size = output_diag_hessian.size();
    if( odh_size != output_size )
    {
        PLERROR("GradNNetLayerModule::bbpropUpdate:"
                " 'output_diag_hessian.size()'\n"
                " should be equal to 'output_size' (%i != %i)\n",
                odh_size, output_size);
    }

    bpropUpdate( input, output, output_gradient );

}

// Propagates back output_gradient and output_diag_hessian
void GradNNetLayerModule::bbpropUpdate(const Vec& input, const Vec& output,
                              Vec&  input_gradient,
                              const Vec& output_gradient,
                              Vec&  input_diag_hessian,
                              const Vec& output_diag_hessian)
{
    bpropUpdate( input, output, input_gradient, output_gradient );
}


// Nothing to forget
void GradNNetLayerModule::forget()
{
    resetWeights();

    if( init_weights.size() !=0 )
    {
        setWeights( init_weights );
    }
    else if (init_weights_random_scale!=0)
    {
        real r = init_weights_random_scale / input_size;
        random_generator->fill_random_uniform(weights.subMatColumns(1,input_size),-r,r);
    }
        

    learning_rate = start_learning_rate;
    step_number = 0;
}


Mat GradNNetLayerModule::getWeights() const
{
    return weights.copy();
}

Vec GradNNetLayerModule::getWeights(int i) const
{
    return weights(i).copy();
}


void GradNNetLayerModule::setWeights(const Mat& the_weights)
{
    int twl = the_weights.length();
    int tww = the_weights.width();
    if( twl != output_size )
    {
        PLERROR( "GradNNetLayerModule::setWeights: number of weights\n"
                 "(the_weights.length()) should be equal to output_size\n"
                 " (%i != %i).\n", twl, output_size );
    }

    if( tww == input_size )
    {
        PLWARNING( "GradNNetLayerModule::setWeights: missing one weight\n"
                   "(the_weights.width() (=%i) != 1+input_size (=%i)).\n"
                   "Assuming it was the bias one, and setting it to '0'.\n"
                   "Be careful: this might not be what you want.\n",
                   tww, 1+input_size );

        weights.subMat( 0, 0, output_size, 1 ) << Vec( output_size );
        weights.subMat( 0, 1, output_size, input_size ) << the_weights;
    }
    else if( tww == 1+input_size )
    {
        weights << the_weights;
    }
    else
    {
        PLERROR( "GradNNetLayerModule::setWeights: the_weights.length()\n"
                 "should be equal to 1+input_size (%i != %i).\n",
                 tww, 1+input_size );
    }
}

void GradNNetLayerModule::setWeights(int i, const Vec& the_weights)
{
    //weights(i) << the_weights;
    int tws = the_weights.size();
    if( tws == input_size )
    {
        PLWARNING( "GradNNetLayerModule::setWeights: missing one weight\n"
                   "(the_weights.size() (=%i) != 1+input_size (=%i)).\n"
                   "Assuming it was the bias one, and setting it to '1'.\n"
                   "Be careful: this might not be what you want.\n",
                   tws, 1+input_size );

        weights(i,0) = 1;
        weights(i).subVec( 1, input_size ) << the_weights;
    }
    else if( tws == input_size+1 )
    {
        weights << the_weights;
    }
    else
    {
        PLERROR( "GradNNetLayerModule::setWeights: the_weights.size()\n"
                 "should be equal to 1+input_size (%i != %i).\n",
                 tws, 1+input_size );
    }
}

void GradNNetLayerModule::resetWeights()
{
    weights.resize( output_size, 1+input_size );
    weights.fill( 0 );
}


// ### Nothing to add here, simply calls build_
void GradNNetLayerModule::build()
{
    inherited::build();
    build_();
}

void GradNNetLayerModule::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    deepCopyField(init_weights, copies);
    deepCopyField(weights, copies);
}

void GradNNetLayerModule::declareOptions(OptionList& ol)
{
    declareOption(ol, "start_learning_rate",
                  &GradNNetLayerModule::start_learning_rate,
                  OptionBase::buildoption,
                  "Learning-rate of stochastic gradient optimization");

    declareOption(ol, "decrease_constant",
                  &GradNNetLayerModule::decrease_constant,
                  OptionBase::buildoption,
                  "Decrease constant of stochastic gradient optimization");

    declareOption(ol, "init_weights", &GradNNetLayerModule::init_weights,
                  OptionBase::buildoption,
                  "Optional initial weights of the neurons (bias on first column,\n"
                  "one row per neuron. If not provided then weights are initialized\n"
                  "according to a uniform distribution (see init_weights_random_scale)\n"
                  "and biases are initialized to 0.\n");

    declareOption(ol, "init_weights_random_scale", &GradNNetLayerModule::init_weights_random_scale,
                  OptionBase::buildoption,
                  "If init_weights is not provided, the weights are initialized randomly\n"
                  "from a uniform in [-r,r], with r = init_weights_random_scale/input_size.\n"
                  "To clear the weights initially, just set this option to 0.");

    declareOption(ol, "L1_penalty_factor", &GradNNetLayerModule::L1_penalty_factor,
                  OptionBase::buildoption,
                  "Optional (default=0) factor of L1 regularization term, i.e.\n"
                  "minimize L1_penalty_factor * sum_{ij} |weights(i,j)| during training.\n");
    
    declareOption(ol, "L2_penalty_factor", &GradNNetLayerModule::L2_penalty_factor,
                  OptionBase::buildoption,
                  "Optional (default=0) factor of L2 regularization term, i.e.\n"
                  "minimize 0.5 * L2_penalty_factor * sum_{ij} weights(i,j)^2 during training.\n");
    

    declareOption(ol, "weights", &GradNNetLayerModule::weights,
                  OptionBase::learntoption,
                  "Input weights of the neurons (bias on first column,"
                  " one row per neuron" );

    inherited::declareOptions(ol);
}

void GradNNetLayerModule::build_()
{
    if( input_size < 0 ) // has not been initialized
    {
        PLERROR("GradNNetLayerModule::build_: 'input_size' < 0 (%i).\n"
                "You should set it to a positive integer.\n", input_size);
    }
    else if( output_size < 0 ) // default to 1 neuron
    {
        PLWARNING("GradNNetLayerModule::build_: 'output_size' is < 0 (%i),\n"
                  " you should set it to a positive integer (the number of"
                  " neurons).\n"
                  " Defaulting to 1.\n", output_size);
        output_size = 1;
    }

    if( weights.size() == 0 )
    {
        resetWeights();
    }

    if (init_weights.size()==0 && init_weights_random_scale!=0 && !random_generator)
        random_generator = new PRandom();
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
