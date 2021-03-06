// -*- C++ -*-

// DiscriminativeRBM.cc
//
// Copyright (C) 2008 Hugo Larochelle
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

// Authors: Hugo Larochelle

/*! \file DiscriminativeRBM.cc */


#define PL_LOG_MODULE_NAME "DiscriminativeRBM"
#include "DiscriminativeRBM.h"
#include <plearn/io/pl_log.h>

#define minibatch_hack 0 // Do we force the minibatch setting? (debug hack)

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    DiscriminativeRBM,
    "Discriminative Restricted Boltzmann Machine classifier.",
    "This classifier supports semi-supervised learning, as well as\n"
    "hybrid generative/discriminative learning. It is based on a\n"
    "Restricted Boltzmann Machine where the visible units contain the\n"
    "the input and the class target.");

///////////////////
// DiscriminativeRBM //
///////////////////
DiscriminativeRBM::DiscriminativeRBM() :
    disc_learning_rate( 0. ),
    disc_decrease_ct( 0. ),
    use_exact_disc_gradient( 0. ),
    gen_learning_weight( 0. ),
    use_multi_conditional_learning( false ),
    semi_sup_learning_weight( 0. ),
    n_classes( -1 ),
    target_weights_L1_penalty_factor( 0. ),
    target_weights_L2_penalty_factor( 0. ),
    do_not_use_discriminative_learning( false ),
    unlabeled_class_index_begin( 0 ),
    n_classes_at_test_time( -1 ),
    n_mean_field_iterations( 1 ),
    gen_learning_every_n_samples( 1 )
{
    random_gen = new PRandom();
}

////////////////////
// declareOptions //
////////////////////
void DiscriminativeRBM::declareOptions(OptionList& ol)
{
    declareOption(ol, "disc_learning_rate", &DiscriminativeRBM::disc_learning_rate,
                  OptionBase::buildoption,
                  "The learning rate used for discriminative learning.\n");

    declareOption(ol, "disc_decrease_ct", &DiscriminativeRBM::disc_decrease_ct,
                  OptionBase::buildoption,
                  "The decrease constant of the discriminative learning rate.\n");

    declareOption(ol, "use_exact_disc_gradient", 
                  &DiscriminativeRBM::use_exact_disc_gradient,
                  OptionBase::buildoption,
                  "Indication that the exact gradient should be used for\n"
                  "discriminative learning (instead of the CD gradient).\n");

    declareOption(ol, "gen_learning_weight", &DiscriminativeRBM::gen_learning_weight,
                  OptionBase::buildoption,
                  "The weight of the generative learning term, for\n"
                  "hybrid discriminative/generative learning.\n");

    declareOption(ol, "use_multi_conditional_learning", 
                  &DiscriminativeRBM::use_multi_conditional_learning,
                  OptionBase::buildoption,
                  "Indication that multi-conditional learning should\n"
                  "be used instead of generative learning.\n");

    declareOption(ol, "semi_sup_learning_weight", 
                  &DiscriminativeRBM::semi_sup_learning_weight,
                  OptionBase::buildoption,
                  "The weight of the semi-supervised learning term, for\n"
                  "unsupervised learning on unlabeled data.\n");

    declareOption(ol, "n_classes", &DiscriminativeRBM::n_classes,
                  OptionBase::buildoption,
                  "Number of classes in the training set.\n"
                  );

    declareOption(ol, "input_layer", &DiscriminativeRBM::input_layer,
                  OptionBase::buildoption,
                  "The input layer of the RBM.\n");

    declareOption(ol, "hidden_layer", &DiscriminativeRBM::hidden_layer,
                  OptionBase::buildoption,
                  "The hidden layer of the RBM.\n");

    declareOption(ol, "connection", &DiscriminativeRBM::connection,
                  OptionBase::buildoption,
                  "The connection weights between the input and hidden layer.\n");

    declareOption(ol, "target_weights_L1_penalty_factor", 
                  &DiscriminativeRBM::target_weights_L1_penalty_factor,
                  OptionBase::buildoption,
                  "Target weights' L1_penalty_factor.\n");

    declareOption(ol, "target_weights_L2_penalty_factor", 
                  &DiscriminativeRBM::target_weights_L2_penalty_factor,
                  OptionBase::buildoption,
                  "Target weights' L2_penalty_factor.\n");

    declareOption(ol, "do_not_use_discriminative_learning", 
                  &DiscriminativeRBM::do_not_use_discriminative_learning,
                  OptionBase::buildoption,
                  "Indication that discriminative learning should not be used.\n");

    declareOption(ol, "unlabeled_class_index_begin", 
                  &DiscriminativeRBM::unlabeled_class_index_begin,
                  OptionBase::buildoption,
                  "The smallest index for the classes of the unlabeled data.\n");

    declareOption(ol, "n_classes_at_test_time", 
                  &DiscriminativeRBM::n_classes_at_test_time,
                  OptionBase::buildoption,
                  "The number of classes to discriminate from during test.\n"
                  "The classes that will be discriminated are indexed\n"
                  "from 0 to n_classes_at_test_time.\n");

    declareOption(ol, "n_mean_field_iterations", 
                  &DiscriminativeRBM::n_mean_field_iterations,
                  OptionBase::buildoption,
                  "Number of mean field iterations for the approximate computation of p(y|x)\n"
                  "for multitask learning.\n");

    declareOption(ol, "gen_learning_every_n_samples", 
                  &DiscriminativeRBM::gen_learning_every_n_samples,
                  OptionBase::buildoption,
                  "Determines the frequency of a generative learning update.\n"
                  "For example, set this option to 100 in order to do an\n"
                  "update every 100 samples. The gen_learning_weight will\n"
                  "then be multiplied by 100.");

    declareOption(ol, "classification_module",
                  &DiscriminativeRBM::classification_module,
                  OptionBase::learntoption,
                  "The module computing the class probabilities.\n"
                  );

    declareOption(ol, "multitask_classification_module",
                  &DiscriminativeRBM::multitask_classification_module,
                  OptionBase::learntoption,
                  "The module approximating the multitask class probabilities.\n"
                  );

    declareOption(ol, "classification_cost",
                  &DiscriminativeRBM::classification_cost,
                  OptionBase::nosave,
                  "The module computing the classification cost function (NLL)"
                  " on top\n"
                  "of classification_module.\n"
                  );

    declareOption(ol, "joint_layer", &DiscriminativeRBM::joint_layer,
                  OptionBase::nosave,
                  "Concatenation of input_layer and the target layer\n"
                  "(that is inside classification_module).\n"
                 );

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

////////////
// build_ //
////////////
void DiscriminativeRBM::build_()
{
    MODULE_LOG << "build_() called" << endl;

    if( inputsize_ > 0 && targetsize_ > 0)
    {
        PLASSERT( n_classes >= 2 );
        PLASSERT( gen_learning_weight >= 0 );
        PLASSERT( semi_sup_learning_weight >= 0 );
        
        build_layers_and_connections();
        build_costs();
    }
}

/////////////////
// build_costs //
/////////////////
void DiscriminativeRBM::build_costs()
{
    cost_names.resize(0);
    
    // build the classification module, its cost and the joint layer
    build_classification_cost();

    int current_index = 0;
    cost_names.append("NLL");
    nll_cost_index = current_index;
    current_index++;
    
    cost_names.append("class_error");
    class_cost_index = current_index;
    current_index++;

    if( targetsize() > 1 )
    {
        cost_names.append("hamming_loss");
        hamming_loss_index = current_index;
        current_index++;
    }
    
    for( int i=0; i<targetsize(); i++ )
    {
        cost_names.append("class_error_" + tostring(i));
        current_index++;
    }

    PLASSERT( current_index == cost_names.length() );
}

//////////////////////////////////
// build_layers_and_connections //
//////////////////////////////////
void DiscriminativeRBM::build_layers_and_connections()
{
    MODULE_LOG << "build_layers_and_connections() called" << endl;

    if( !input_layer )
        PLERROR("In DiscriminativeRBM::build_layers_and_connections(): "
                "input_layer must be provided");
    if( !hidden_layer )
        PLERROR("In DiscriminativeRBM::build_layers_and_connections(): "
                "hidden_layer must be provided");

    if( !connection )
        PLERROR("DiscriminativeRBM::build_layers_and_connections(): \n"
                "connection must be provided");

    if( connection->up_size != hidden_layer->size ||
        connection->down_size != input_layer->size )
        PLERROR("DiscriminativeRBM::build_layers_and_connections(): \n"
                "connection's size (%d x %d) should be %d x %d",
                connection->up_size, connection->down_size,
                hidden_layer->size, input_layer->size);

    if( inputsize_ >= 0 )
        PLASSERT( input_layer->size == inputsize() );

    input_gradient.resize( inputsize() );
    class_output.resize( n_classes );
    before_class_output.resize( n_classes );
    class_gradient.resize( n_classes );

    target_one_hot.resize( n_classes );

    disc_pos_down_val.resize( inputsize() + n_classes );
    disc_pos_up_val.resize( hidden_layer->size );
    disc_neg_down_val.resize( inputsize() + n_classes );
    disc_neg_up_val.resize( hidden_layer->size );
  
    gen_pos_down_val.resize( inputsize() + n_classes );
    gen_pos_up_val.resize( hidden_layer->size );
    gen_neg_down_val.resize( inputsize() + n_classes );
    gen_neg_up_val.resize( hidden_layer->size );

    semi_sup_pos_down_val.resize( inputsize() + n_classes );
    semi_sup_pos_up_val.resize( hidden_layer->size );
    semi_sup_neg_down_val.resize( inputsize() + n_classes );
    semi_sup_neg_up_val.resize( hidden_layer->size );



    if( !input_layer->random_gen )
    {
        input_layer->random_gen = random_gen;
        input_layer->forget();
    }

    if( !hidden_layer->random_gen )
    {
        hidden_layer->random_gen = random_gen;
        hidden_layer->forget();
    }

    if( !connection->random_gen )
    {
        connection->random_gen = random_gen;
        connection->forget();
    }
}

///////////////////////////////
// build_classification_cost //
///////////////////////////////
void DiscriminativeRBM::build_classification_cost()
{
    MODULE_LOG << "build_classification_cost() called" << endl;

    if( targetsize() == 1 )
    {
        if (!classification_module ||
            classification_module->target_layer->size != n_classes ||
            classification_module->last_layer != hidden_layer || 
            classification_module->previous_to_last != connection )
        {
            // We need to (re-)create 'last_to_target', and thus the classification
            // module too.
            // This is not systematically done so that the learner can be
            // saved and loaded without losing learned parameters.
            last_to_target = new RBMMatrixConnection();
            last_to_target->up_size = hidden_layer->size;
            last_to_target->down_size = n_classes;
            last_to_target->L1_penalty_factor = target_weights_L1_penalty_factor;
            last_to_target->L2_penalty_factor = target_weights_L2_penalty_factor;
            last_to_target->random_gen = random_gen;
            last_to_target->build();
            
            target_layer = new RBMMultinomialLayer();
            target_layer->size = n_classes;
            target_layer->random_gen = random_gen;
            target_layer->build();
            
            classification_module = new RBMClassificationModule();
            classification_module->previous_to_last = connection;
            classification_module->last_layer = hidden_layer;
            classification_module->last_to_target = last_to_target;
            classification_module->target_layer = 
                dynamic_cast<RBMMultinomialLayer*>((RBMLayer*) target_layer);
            classification_module->random_gen = random_gen;
            classification_module->build();
        }

        classification_cost = new NLLCostModule();
        classification_cost->input_size = n_classes;
        classification_cost->target_size = 1;
        classification_cost->build();
        
        last_to_target = classification_module->last_to_target;
        last_to_target_connection = 
            (RBMMatrixConnection*) classification_module->last_to_target;
        target_layer = classification_module->target_layer;
        joint_connection = classification_module->joint_connection;
        
        joint_layer = new RBMMixedLayer();
        joint_layer->sub_layers.resize( 2 );
        joint_layer->sub_layers[0] = input_layer;
        joint_layer->sub_layers[1] = target_layer;
        joint_layer->random_gen = random_gen;
        joint_layer->build();
        
        if( unlabeled_class_index_begin != 0 )
        {
            unlabeled_class_output.resize( n_classes - unlabeled_class_index_begin );
            PP<RBMMultinomialLayer> sub_layer = new RBMMultinomialLayer();
            sub_layer->bias = target_layer->bias.subVec(
                unlabeled_class_index_begin,
                n_classes - unlabeled_class_index_begin);
            sub_layer->size = n_classes - unlabeled_class_index_begin;
            sub_layer->random_gen = random_gen;
            sub_layer->build();
            
            PP<RBMMatrixConnection> sub_connection = new RBMMatrixConnection();
            sub_connection->weights = last_to_target->weights.subMatColumns(
                unlabeled_class_index_begin,
                n_classes - unlabeled_class_index_begin);
            sub_connection->up_size = hidden_layer->size;
            sub_connection->down_size = n_classes - unlabeled_class_index_begin;
            sub_connection->random_gen = random_gen;
            sub_connection->build();
            
            unlabeled_classification_module = new RBMClassificationModule();
            unlabeled_classification_module->previous_to_last = connection;
            unlabeled_classification_module->last_layer = hidden_layer;
            unlabeled_classification_module->last_to_target = sub_connection;
            unlabeled_classification_module->target_layer = sub_layer;
            unlabeled_classification_module->random_gen = random_gen;
            unlabeled_classification_module->build();
        }
        
        if( n_classes_at_test_time > 0 && n_classes_at_test_time != n_classes )
        {
            test_time_class_output.resize( n_classes_at_test_time ); 
            PP<RBMMultinomialLayer> sub_layer = new RBMMultinomialLayer();
            sub_layer->bias = target_layer->bias.subVec(
                0, n_classes_at_test_time );
            sub_layer->size = n_classes_at_test_time;
            sub_layer->random_gen = random_gen;
            sub_layer->build();
            
            PP<RBMMatrixConnection> sub_connection = new RBMMatrixConnection();
            sub_connection->weights = last_to_target->weights.subMatColumns(
                0, n_classes_at_test_time );
            sub_connection->up_size = hidden_layer->size;
            sub_connection->down_size = n_classes_at_test_time;
            sub_connection->random_gen = random_gen;
            sub_connection->build();
            test_time_classification_module = new RBMClassificationModule();
            test_time_classification_module->previous_to_last = connection;
            test_time_classification_module->last_layer = hidden_layer;
            test_time_classification_module->last_to_target = sub_connection;
            test_time_classification_module->target_layer = sub_layer;
            test_time_classification_module->random_gen = random_gen;
            test_time_classification_module->build();
        }
        else
        {
            test_time_classification_module = 0;
        }
    }
    else
    {
        if( n_classes != targetsize() )
            PLERROR("In DiscriminativeRBM::build_classification_cost(): "
                    "n_classes should be equal to targetsize()");
        
        // Multitask setting
        if (!multitask_classification_module ||
            multitask_classification_module->target_layer->size != n_classes ||
            multitask_classification_module->last_layer != hidden_layer || 
            multitask_classification_module->previous_to_last != connection )
        {
            // We need to (re-)create 'last_to_target', and thus the 
            // multitask_classification module too.
            // This is not systematically done so that the learner can be
            // saved and loaded without losing learned parameters.
            last_to_target = new RBMMatrixConnection();
            last_to_target->up_size = hidden_layer->size;
            last_to_target->down_size = n_classes;
            last_to_target->L1_penalty_factor = target_weights_L1_penalty_factor;
            last_to_target->L2_penalty_factor = target_weights_L2_penalty_factor;
            last_to_target->random_gen = random_gen;
            last_to_target->build();
            
            target_layer = new RBMBinomialLayer();
            target_layer->size = n_classes;
            target_layer->random_gen = random_gen;
            target_layer->build();
            
            multitask_classification_module = 
                new RBMMultitaskClassificationModule();
            multitask_classification_module->previous_to_last = connection;
            multitask_classification_module->last_layer = hidden_layer;
            multitask_classification_module->last_to_target = last_to_target;
            multitask_classification_module->target_layer = 
                dynamic_cast<RBMBinomialLayer*>((RBMLayer*) target_layer);
            multitask_classification_module->fprop_outputs_activation = true;
            multitask_classification_module->n_mean_field_iterations = n_mean_field_iterations;
            multitask_classification_module->random_gen = random_gen;
            multitask_classification_module->build();
        }

        last_to_target = multitask_classification_module->last_to_target;
        last_to_target_connection = 
            (RBMMatrixConnection*) multitask_classification_module->last_to_target;
        target_layer = multitask_classification_module->target_layer;
        joint_connection = multitask_classification_module->joint_connection;
        
        joint_layer = new RBMMixedLayer();
        joint_layer->sub_layers.resize( 2 );
        joint_layer->sub_layers[0] = input_layer;
        joint_layer->sub_layers[1] = target_layer;
        joint_layer->random_gen = random_gen;
        joint_layer->build();
        
        if( unlabeled_class_index_begin != 0 )
            PLERROR("In DiscriminativeRBM::build_classification_cost(): "
                "can't use unlabeled_class_index_begin != 0 in multitask setting");
        
        if( n_classes_at_test_time > 0 && n_classes_at_test_time != n_classes )
            PLERROR("In DiscriminativeRBM::build_classification_cost(): "
                "can't use n_classes_at_test_time in multitask setting");
    }
}

///////////
// build //
///////////
void DiscriminativeRBM::build()
{
    inherited::build();
    build_();
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void DiscriminativeRBM::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    deepCopyField(input_layer, copies);
    deepCopyField(hidden_layer, copies);
    deepCopyField(connection, copies);
    deepCopyField(classification_module, copies);
    deepCopyField(multitask_classification_module, copies);
    deepCopyField(cost_names, copies);
    deepCopyField(classification_cost, copies);
    deepCopyField(joint_layer, copies);
    deepCopyField(last_to_target, copies);
    deepCopyField(last_to_target_connection, copies);
    deepCopyField(joint_connection, copies);
    deepCopyField(target_layer, copies);
    deepCopyField(unlabeled_classification_module, copies);
    deepCopyField(test_time_classification_module, copies);
    deepCopyField(target_one_hot, copies);
    deepCopyField(disc_pos_down_val, copies);
    deepCopyField(disc_pos_up_val, copies);
    deepCopyField(disc_neg_down_val, copies);
    deepCopyField(disc_neg_up_val, copies);
    deepCopyField(gen_pos_down_val, copies);
    deepCopyField(gen_pos_up_val, copies);
    deepCopyField(gen_neg_down_val, copies);
    deepCopyField(gen_neg_up_val, copies);
    deepCopyField(semi_sup_pos_down_val, copies);
    deepCopyField(semi_sup_pos_up_val, copies);
    deepCopyField(semi_sup_neg_down_val, copies);
    deepCopyField(semi_sup_neg_up_val, copies);
    deepCopyField(input_gradient, copies);
    deepCopyField(class_output, copies);
    deepCopyField(before_class_output, copies);
    deepCopyField(unlabeled_class_output, copies);
    deepCopyField(test_time_class_output, copies);
    deepCopyField(class_gradient, copies);
}


////////////////
// outputsize //
////////////////
int DiscriminativeRBM::outputsize() const
{
    return n_classes_at_test_time > 0 ? n_classes_at_test_time : n_classes;
}

////////////
// forget //
////////////
void DiscriminativeRBM::forget()
{
    inherited::forget();

    input_layer->forget();
    hidden_layer->forget();
    connection->forget();
    if( targetsize() > 1 )
    {
        multitask_classification_module->forget();
    }
    else
    {
        classification_cost->forget();
        classification_module->forget();
    }
}

///////////
// train //
///////////
void DiscriminativeRBM::train()
{
    MODULE_LOG << "train() called " << endl;

    MODULE_LOG << "stage = " << stage
        << ", target nstages = " << nstages << endl;

    PLASSERT( train_set );

    Vec input( inputsize() );
    Vec target( targetsize() );
    int target_index = -1;
    real weight; 

    real nll_cost; 
    real class_error;
    TVec<string> train_cost_names = getTrainCostNames() ;
    Vec train_costs( train_cost_names.length() );
    train_costs.fill(MISSING_VALUE) ;

    int nsamples = train_set->length();
    int init_stage = stage;
    if( !initTrain() )
    {
        MODULE_LOG << "train() aborted" << endl;
        return;
    }

    PP<ProgressBar> pb;

    // clear stats of previous epoch
    train_stats->forget();

    if( report_progress )
        pb = new ProgressBar( "Training "
                              + classname(),
                              nstages - stage );

    //! This makes sure that the samples on which generative learning 
    //! is done changes from one epoch to another
    int offset = (int)round(stage/nstages) % gen_learning_every_n_samples;

    for( ; stage<nstages ; stage++ )
    {
        train_set->getExample(stage%nsamples, input, target, weight);
        if( pb )
            pb->update( stage - init_stage + 1 );

        if( targetsize() == 1 )
        {
            target_one_hot.clear();
            if( !is_missing(target[0]) && (target[0] >= 0) )
            {
                target_index = (int)round( target[0] );
                target_one_hot[ target_index ] = 1;
            }
        }
        else
        {
            target_one_hot << target;
        }

        // Get CD stats...

        // ... for discriminative learning
        if( !do_not_use_discriminative_learning && 
            !use_exact_disc_gradient && 
            ( ( !is_missing(target[0]) && (target[0] >= 0) ) || targetsize() > 1 ) )
        {
            // Positive phase

            // Clamp visible units
            target_layer->sample << target_one_hot;
            input_layer->sample << input ;

            // Up pass
            joint_connection->setAsDownInput( joint_layer->sample );
            hidden_layer->getAllActivations( joint_connection );
            hidden_layer->computeExpectation();
            hidden_layer->generateSample();

            disc_pos_down_val << joint_layer->sample;
            disc_pos_up_val << hidden_layer->expectation;

            // Negative phase

            // Down pass
            last_to_target_connection->setAsUpInput( hidden_layer->sample );
            target_layer->getAllActivations( last_to_target_connection );
            target_layer->computeExpectation();
            target_layer->generateSample();

            // Up pass
            joint_connection->setAsDownInput( joint_layer->sample );
            hidden_layer->getAllActivations( joint_connection );
            hidden_layer->computeExpectation();

            disc_neg_down_val << joint_layer->sample;
            disc_neg_up_val << hidden_layer->expectation;
        }

        // ... for generative learning
        if( (stage + offset) % gen_learning_every_n_samples == 0 )
        {            
            if( ( ( !is_missing(target[0]) && (target[0] >= 0) ) || targetsize() > 1 ) && 
                gen_learning_weight > 0 )
            {
                // Positive phase
                if( !use_exact_disc_gradient && !do_not_use_discriminative_learning )
                {
                    // Use previous computations
                    gen_pos_down_val << disc_pos_down_val;
                    gen_pos_up_val << disc_pos_up_val;

                    hidden_layer->setExpectation( gen_pos_up_val );
                    hidden_layer->generateSample();
                }
                else
                {
                    // Clamp visible units
                    target_layer->sample << target_one_hot;
                    input_layer->sample << input ;
                
                    // Up pass
                    joint_connection->setAsDownInput( joint_layer->sample );
                    hidden_layer->getAllActivations( joint_connection );
                    hidden_layer->computeExpectation();
                    hidden_layer->generateSample();
                
                    gen_pos_down_val << joint_layer->sample;
                    gen_pos_up_val << hidden_layer->expectation;
                }

                // Negative phase

                if( !use_multi_conditional_learning )
                {
                    // Down pass
                    joint_connection->setAsUpInput( hidden_layer->sample );
                    joint_layer->getAllActivations( joint_connection );
                    joint_layer->computeExpectation();
                    joint_layer->generateSample();
                
                    // Up pass
                    joint_connection->setAsDownInput( joint_layer->sample );
                    hidden_layer->getAllActivations( joint_connection );
                    hidden_layer->computeExpectation();
                }
                else
                {
                    target_layer->sample << target_one_hot;

                    // Down pass
                    connection->setAsUpInput( hidden_layer->sample );
                    input_layer->getAllActivations( connection );
                    input_layer->computeExpectation();
                    input_layer->generateSample();
                
                    // Up pass
                    joint_connection->setAsDownInput( joint_layer->sample );
                    hidden_layer->getAllActivations( joint_connection );
                    hidden_layer->computeExpectation(); 
                }

                gen_neg_down_val << joint_layer->sample;
                gen_neg_up_val << hidden_layer->expectation;

            }
        }

        // ... and for semi-supervised learning
        if( targetsize() > 1 && semi_sup_learning_weight > 0 )
            PLERROR("DiscriminativeRBM::train(): semi-supervised learning "
                "is not implemented yet for multi-task learning.");

        if( ( is_missing(target[0]) || target[0] < 0 ) && semi_sup_learning_weight > 0 )
        {
            // Positive phase

            // Clamp visible units and sample from p(y|x)
            if( unlabeled_classification_module )
            {
                unlabeled_classification_module->fprop( input,
                                                        unlabeled_class_output );
                class_output.clear();
                class_output.subVec( unlabeled_class_index_begin,
                                     n_classes - unlabeled_class_index_begin )
                    << unlabeled_class_output;
            }
            else
            {
                classification_module->fprop( input,
                                              class_output );
            }
            target_layer->setExpectation( class_output );
            target_layer->generateSample();            
            input_layer->sample << input ;
            
             // Up pass
            joint_connection->setAsDownInput( joint_layer->sample );
            hidden_layer->getAllActivations( joint_connection );
            hidden_layer->computeExpectation();
            hidden_layer->generateSample();
            
            semi_sup_pos_down_val << joint_layer->sample;
            semi_sup_pos_up_val << hidden_layer->expectation;
            
            // Negative phase

            // Down pass
            joint_connection->setAsUpInput( hidden_layer->sample );
            joint_layer->getAllActivations( joint_connection );
            joint_layer->computeExpectation();
            joint_layer->generateSample();
            
            // Up pass
            joint_connection->setAsDownInput( joint_layer->sample );
            hidden_layer->getAllActivations( joint_connection );
            hidden_layer->computeExpectation();

            semi_sup_neg_down_val << joint_layer->sample;
            semi_sup_neg_up_val << hidden_layer->expectation;
        }

        if( train_set->weightsize() == 0 )
            setLearningRate( disc_learning_rate / (1. + disc_decrease_ct * stage ));
        else
            setLearningRate( weight * disc_learning_rate / (1. + disc_decrease_ct * stage ));
        // Get gradient and update

        if( !do_not_use_discriminative_learning && 
            use_exact_disc_gradient && 
            ( ( !is_missing(target[0]) && (target[0] >= 0)  ) || targetsize() > 1 ) )
        {
            if( targetsize() == 1)
            {
                PLASSERT( target_index >= 0 );
                classification_module->fprop( input, class_output );
                // This doesn't work. gcc bug?
                //classification_cost->fprop( class_output, target, nll_cost );
                classification_cost->CostModule::fprop( class_output, target,
                                                        nll_cost );
                
                class_error =  ( argmax(class_output) == target_index ) ? 0: 1;  
                train_costs[nll_cost_index] = nll_cost;
                train_costs[class_cost_index] = class_error;
                
                classification_cost->bpropUpdate( class_output, target, nll_cost,
                                                  class_gradient );
                
                classification_module->bpropUpdate( input,  class_output,
                                                    input_gradient, class_gradient );
                
                train_stats->update( train_costs );
            }
            else
            {
                multitask_classification_module->fprop( input, before_class_output );
                // This doesn't work. gcc bug?
                //multitask_classification_cost->fprop( class_output, target, 
                //                                      nll_cost );
                //multitask_classification_cost->CostModule::fprop( class_output, 
                //                                                  target,
                //                                                  nll_cost );
                
                target_layer->fprop( before_class_output, class_output );
                target_layer->activation << before_class_output;
                target_layer->activation += target_layer->bias;
                target_layer->setExpectation( class_output );
                nll_cost = target_layer->fpropNLL( target );
                
                train_costs.clear();
                train_costs[nll_cost_index] = nll_cost;

                for( int task=0; task<targetsize(); task++)
                {
                    if( class_output[task] > 0.5 && target[task] != 1)
                    {
                        train_costs[ hamming_loss_index ]++;
                        train_costs[ hamming_loss_index + task + 1 ] = 1;
                    }
                    
                    if( class_output[task] <= 0.5 && target[task] != 0)
                    {
                        train_costs[ hamming_loss_index ]++;
                        train_costs[ hamming_loss_index + task + 1 ] = 1;
                    }
                }

                if( train_costs[ hamming_loss_index ] > 0 )
                    train_costs[ class_cost_index ] = 1;

                train_costs[ hamming_loss_index ] /= targetsize();
                
                //multitask_classification_cost->bpropUpdate( 
                //    class_output, target, nll_cost,
                //    class_gradient );
                
                class_gradient.clear();
                target_layer->bpropNLL( target, nll_cost, class_gradient );
                target_layer->update( class_gradient );

                multitask_classification_module->bpropUpdate( 
                    input,  before_class_output,
                    input_gradient, class_gradient );
                
                train_stats->update( train_costs );
            }
        }

        // CD Updates
        if( !do_not_use_discriminative_learning && 
            !use_exact_disc_gradient && ( !is_missing(target[0]) && (target[0] >= 0) ) )
        {
            joint_layer->update( disc_pos_down_val, disc_neg_down_val );
            hidden_layer->update( disc_pos_up_val, disc_neg_up_val );
            joint_connection->update( disc_pos_down_val, disc_pos_up_val,
                                disc_neg_down_val, disc_neg_up_val);
        }

        if( (stage + offset) % gen_learning_every_n_samples == 0 )
        { 
            if( ( !is_missing(target[0]) && (target[0] >= 0) ) && gen_learning_weight > 0 )
            {
                if( train_set->weightsize() == 0 )
                    setLearningRate( gen_learning_every_n_samples * gen_learning_weight * disc_learning_rate / 
                                     (1. + disc_decrease_ct * stage ));
                else
                    setLearningRate( weight * gen_learning_every_n_samples * gen_learning_weight * disc_learning_rate / 
                                     (1. + disc_decrease_ct * stage ));
                    
                joint_layer->update( gen_pos_down_val, gen_neg_down_val );
                hidden_layer->update( gen_pos_up_val, gen_neg_up_val );
                joint_connection->update( gen_pos_down_val, gen_pos_up_val,
                                          gen_neg_down_val, gen_neg_up_val);
            }
        }            

        if( ( is_missing(target[0]) || (target[0] < 0)  ) && semi_sup_learning_weight > 0 )
        {
            if( train_set->weightsize() == 0 )
                setLearningRate( semi_sup_learning_weight * disc_learning_rate / 
                                 (1. + disc_decrease_ct * stage ));
            else
                setLearningRate( weight * semi_sup_learning_weight * disc_learning_rate / 
                                 (1. + disc_decrease_ct * stage ));
                
            joint_layer->update( semi_sup_pos_down_val, semi_sup_neg_down_val );
            hidden_layer->update( semi_sup_pos_up_val, semi_sup_neg_up_val );
            joint_connection->update( semi_sup_pos_down_val, semi_sup_pos_up_val,
                                semi_sup_neg_down_val, semi_sup_neg_up_val);
        }

    }
    
    train_stats->finalize();
}


///////////////////
// computeOutput //
///////////////////
void DiscriminativeRBM::computeOutput(const Vec& input, Vec& output) const
{
    // Compute the output from the input.
    output.resize(0);
    if( targetsize() == 1 )
    {
        if( test_time_classification_module )
        {
            test_time_classification_module->fprop( input,
                                                    output );
        }
        else
        {
            classification_module->fprop( input,
                                          output );
        }
    }
    else
    {
        multitask_classification_module->fprop( input,
                                                output );
    }
}


void DiscriminativeRBM::computeCostsFromOutputs(const Vec& input, const Vec& output,
                                           const Vec& target, Vec& costs) const
{

    // Compute the costs from *already* computed output.
    costs.resize( cost_names.length() );
    costs.fill( MISSING_VALUE );

    if( targetsize() == 1 )
    {
        if( !is_missing(target[0]) && (target[0] >= 0) )
        {
            //classification_cost->fprop( output, target, costs[nll_cost_index] );
            //classification_cost->CostModule::fprop( output, target, costs[nll_cost_index] );
            costs[nll_cost_index] = -pl_log(output[(int) round(target[0])]);
            costs[class_cost_index] =
                (argmax(output) == (int) round(target[0]))? 0 : 1;
        }
    }
    else
    {
        costs.clear();

        // This doesn't work. gcc bug?
        //multitask_classification_cost->fprop( output, target, 
        //                                      costs[nll_cost_index] );
        //multitask_classification_cost->CostModule::fprop( output, 
        //                                                  target,
        //                                                  nll_cost );

        target_layer->fprop( output, class_output );
        target_layer->activation << output;
        target_layer->activation += target_layer->bias;
        target_layer->setExpectation( class_output );
        costs[ nll_cost_index ] = target_layer->fpropNLL( target );


        for( int task=0; task<targetsize(); task++)
        {
            if( class_output[task] > 0.5 && target[task] != 1)
            {
                costs[ hamming_loss_index ]++;
                costs[ hamming_loss_index + task + 1 ] = 1;
            }
            
            if( class_output[task] <= 0.5 && target[task] != 0)
            {
                costs[ hamming_loss_index ]++;
                costs[ hamming_loss_index + task + 1 ] = 1;
            }
        }
        
        if( costs[ hamming_loss_index ] > 0 )
            costs[ class_cost_index ] = 1;
        
        costs[ hamming_loss_index ] /= targetsize();
    }
}

TVec<string> DiscriminativeRBM::getTestCostNames() const
{
    // Return the names of the costs computed by computeCostsFromOutputs
    // (these may or may not be exactly the same as what's returned by
    // getTrainCostNames).

    return cost_names;
}

TVec<string> DiscriminativeRBM::getTrainCostNames() const
{
    return cost_names;
}


//#####  Helper functions  ##################################################

void DiscriminativeRBM::setLearningRate( real the_learning_rate )
{
    input_layer->setLearningRate( the_learning_rate );
    hidden_layer->setLearningRate( the_learning_rate );
    connection->setLearningRate( the_learning_rate );
    target_layer->setLearningRate( the_learning_rate );
    last_to_target->setLearningRate( the_learning_rate );
    if( targetsize() == 1)
        classification_cost->setLearningRate( the_learning_rate );
    //else
    //    multitask_classification_cost->setLearningRate( the_learning_rate );
    //classification_module->setLearningRate( the_learning_rate );
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
