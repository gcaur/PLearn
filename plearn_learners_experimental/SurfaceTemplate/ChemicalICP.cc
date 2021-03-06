// -*- C++ -*-

// ChemicalICP.cc
//
// Copyright (C) 2006 Pascal Lamblin
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

// Authors: Pascal Lamblin

/*! \file ChemicalICP.cc */

#include "ChemicalICP.h"
#include <plearn/base/stringutils.h>
#include <plearn/io/openFile.h>
#include <plearn/var/UnaryVariable.h>
#include <plearn/var/VarColumnsVariable.h>
#include <plearn/io/pl_log.h>
#include "geometry.h"

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    ChemicalICP,
    "Iterative Closest Points (ICP) aligner.",
    ""
);

ChemicalICP::ChemicalICP():
      mol_feat_indices( new UnaryVariable() ),
      template_feat_indices( new UnaryVariable() ),
      matching_neighbors( new UnaryVariable() ),
      weighting_method( "features_sigmoid" ),
      weighting_params( Vec(2,1) ),
      matching_method( "exhaustive" ),
      memory( "none" ),
      initial_angles_step( 0 ),
      max_iter( 50 ),
      error_t( 0 ),
      angle_t( 0.5 ),
      trans_t( 0 ),
      rotation( 3, 3 ),
      translation( 3 ),
      all_mol_features( 0 ),
      all_template_features( 0 ),
      all_template_feat_dev( 0 ),
      mol_coordinates( 0 ),
      used_mol_features( new VarColumnsVariable( all_mol_features,
                                                 mol_feat_indices ) ),
      template_coordinates( 0 ),
      template_geom_dev( 0 ),
      used_template_features( new VarColumnsVariable(all_template_features,
                                                     template_feat_indices) ),
      used_template_feat_dev( new VarColumnsVariable(all_template_feat_dev,
                                                     template_feat_indices) )
{}

ChemicalICP::ChemicalICP( const MolTemplate& the_template,
                          const Mol& the_molecule,
                          const TVec<string>& the_feature_names,
                          string the_weighting_method,
                          const Var& the_weighting_params,
                          string the_matching_method ):
      mol_feat_indices( new UnaryVariable() ),
      template_feat_indices( new UnaryVariable() ),
      matching_neighbors( new UnaryVariable() ),
      mol_template(the_template),
      molecule(the_molecule),
      feature_names(the_feature_names),
      weighting_method(the_weighting_method),
      weighting_params(the_weighting_params),
      matching_method( the_matching_method ),
      memory( "none" ),
      initial_angles_step( 0 ),
      max_iter( 50 ),
      error_t( 0 ),
      angle_t( 0.5 ),
      trans_t( 0 ),
      rotation( 3, 3 ),
      translation( 3 ),
      all_mol_features( 0 ), // calls SourceVariable()
      all_template_features( 0 ),
      all_template_feat_dev( 0 ),
      mol_coordinates( 0 ),
      used_mol_features( new VarColumnsVariable( all_mol_features,
                                                 mol_feat_indices ) ),
      template_coordinates( 0 ),
      template_geom_dev( 0 ),
      used_template_features( new VarColumnsVariable(all_template_features,
                                                     template_feat_indices) ),
      used_template_feat_dev( new VarColumnsVariable(all_template_feat_dev,
                                                     template_feat_indices) )
{
//pout << "Big constructor called" << endl;
    build();
}

void ChemicalICP::setMolecule( const Mol& the_molecule )
{
    molecule = the_molecule;
    computeUsedFeatures();
    computeVariables();

    if( matching_method == "exhaustive" )
        cacheFeatureDistances();
}

void ChemicalICP::computeUsedFeatures()
{
    // compute intersection between mol_template->feature_names and
    // feature_names
    TVec<string> template_names = mol_template->feature_names;
    TVec<string> common_names;
    TVec<int> common_indices;

    if( feature_names.length() == 0 )
    {
        common_names = template_names.copy();
        common_indices = TVec<int>( 0, template_names.length() - 1, 1 );
    }
    else if( feature_names[0] == "none" )
    {
        used_feat_names.resize( 0 );
        mol_feat_indices->resize( 0, 0 );
        template_feat_indices->resize( 0, 0 );
        return;
    }
    else
    {
        common_names.resize(0, feature_names.length()); // to have some space
        common_indices.resize(0, feature_names.length());
        TVec<int> indices = feature_names.find( template_names );

        for( int i=0 ; i<indices.length() ; i++ )
        {
            if( indices[i] >= 0 )
            {
                // template_names[i] is present in feature_names
                common_names.append( template_names[i] );
                common_indices.append( i );
            }
        }
    }

    // then compute intersection with molecule->feature_names
    TVec<string> mol_names = molecule->feature_names;
    used_feat_names.resize(0, common_names.length());

    // storage for indices
    Vec template_feat_indices_;
    Vec mol_feat_indices_;

    TVec<int> indices = mol_names.find( common_names );
    for( int i=0 ; i<indices.length() ; i++ )
    {
        if( indices[i] >= 0 )
        {
            // common_names[i] is present in mol_names at position indices[i]
            used_feat_names.append( common_names[i] );
            template_feat_indices_.append( common_indices[i] );
            mol_feat_indices_.append( indices[i] );
        }
    }

    // update Var's value
    template_feat_indices->defineValueLocation(
        template_feat_indices_.toMat( template_feat_indices_.size(), 1 ) );

    mol_feat_indices->defineValueLocation(
        mol_feat_indices_.toMat( mol_feat_indices_.size(), 1 ) );
}

void ChemicalICP::computeVariables()
{
    // make mol_coordinates point to the new molecule's coordinates
    mol_coordinates->defineValueLocation( molecule->coordinates );

    // make mol_features point to the new molecule's features
    all_mol_features->defineValueLocation( molecule->features );

    // update used_... Var's so they have the right size
    used_mol_features->sizefprop();
    used_template_features->sizefprop();
    used_template_feat_dev->sizefprop();
}

void ChemicalICP::cacheFeatureDistances()
{
//pout << "cacheFeatureDistances()" << endl;
    Mat t_features = used_template_features->matValue;
    Mat t_feat_dev = used_template_feat_dev->matValue;
    Mat m_features = used_mol_features->matValue;

    int n_template_points = t_features.length();
    int n_mol_points = m_features.length();
    int n_features = m_features.width();

    feat_distances2.resize( n_template_points, n_mol_points );
    feat_distances2.fill(0);

    for( int i=0 ; i<n_template_points ; i++ )
    {
        for( int j=0 ; j<n_mol_points ; j++ )
        {
            for( int k=0 ; k<n_features ; k++ )
            {
                real diff = (t_features(i, k) - m_features(j, k))
                                / t_feat_dev(i, k);
                feat_distances2(i, j) += diff * diff;
            }
        }
    }
/*pout << "t_features(0) = " << t_features(0) << endl;
pout << "t_feat_dev(0) = " << t_feat_dev(0) << endl;
pout << "feat_distances2(0) = " << feat_distances2(0) << endl;*/
}

///////////////////////////////
// forgetMemorizedAlignments //
///////////////////////////////
void ChemicalICP::forgetMemorizedAlignments()
{
    mem_aligned_molecules.resize(0);
    mem_error.resize(0);
    mem_rotation.resize(0);
    mem_translation.resize(0);
    mem_matching.resize(0);
    mem_count.resize(0);
}

/////////
// run //
/////////
void ChemicalICP::run()
{
    if( initial_angles_list.length() > 0 )
    {
        if( initial_angles_step > 0 )
        {
            PLWARNING( "ChemicalICP::run - both 'initial_angles_step' and"
                       " 'initial_angles_list'\n"
                       "are provided. Setting 'initial_angles_step' to 0.\n" );
            initial_angles_step = 0;
        }
    }
    else
    {
        if( fast_is_equal( initial_angles_step, 0. ) )
            initial_angles_step = 360;

        initial_angles_list.resize( 0, 3 );
        for( real rx=0. ; rx<360. ; rx += initial_angles_step )
            for( real ry=0. ; ry<360. ; ry += initial_angles_step )
                for( real rz=0. ; rz<180. ; rz += initial_angles_step )
                {
                    Vec angles( 3 );
                    angles[0] = rx;
                    angles[1] = ry;
                    angles[2] = rz;
                    initial_angles_list.appendRow( angles );
                }
    }
    EXTREME_NAMED_LOG("ICP+NNet") << "begin run()" << endl;

    // Only perform the alignment if we do not just obtain the result from a
    // previously memorized alignment.
    bool realign = true;
    int find_mol = -1;
    if (memory != "none") {
        find_mol = mem_aligned_molecules.find(molecule);
        if (find_mol == -1) {
            // This molecule had not been previously aligned.
            find_mol = mem_aligned_molecules.length();
            mem_aligned_molecules.append(molecule);
            // Dummy additions just to resize the vectors correctly.
            mem_error.append(0);
            mem_rotation.append(Mat());
            mem_translation.append(Vec());
            mem_matching.append(TVec<int>());
            mem_count.append(0);
        }
    }

    if (every_x > 0) {
        PLASSERT( find_mol >= 0 );
        if (mem_count[find_mol] % every_x != 0)
            realign = false;
    }

    if (realign) {

    int n_points = mol_template->n_points();
    if( n_points < 3 )
        PLERROR( "ChemicalICP::run() - not enough points in template (%d).\n",
                 n_points );

    Mat best_rotation(3, 3);
    Vec best_translation(3);
    TVec<int> best_matching( n_points );
    Vec best_weights( n_points );
    real best_error = REAL_MAX;

    // transformed template coordinates
    Mat tr_template_coords( n_points, 3 );

    // coordinates of molecule points matched to template ones
    Mat matched_mol_coords( n_points, 3 );

    int n_initial_angles = initial_angles_list.length();
    EXTREME_NAMED_LOG("ICP+NNet") << n_initial_angles << " initial angles to try" << endl;
    for( int i=0 ; i<n_initial_angles ; i++ )
    {
		EXTREME_NAMED_LOG("ICP+NNet") << "global iteration number " << i << endl;
        // initialization
        rotation = rotationMatrixFromAngles( initial_angles_list(i) );
        translation.fill(0);
        matching = TVec<int>( n_points, -1 );
        weights = Vec( n_points );

        int n_iter = 0;
        real delta_rot_length = REAL_MAX;
        real delta_trans_length = REAL_MAX;

        applyGeomTransformation( rotation, translation,
                                 template_coordinates->matValue,
                                 tr_template_coords );
		EXTREME_NAMED_LOG("ICPDEBUG") << "beginning ICP:" << endl
			<< "rotation = " << endl << rotation << endl
		<< "translation = " << translation << endl
		<< "template_coordinates->matValue = " << endl
		<< template_coordinates->matValue << endl
		<< "tr_template_coords = " << endl << tr_template_coords << endl
		<< endl;
        // main loop
        do
        {
			EXTREME_NAMED_LOG("ICPDEBUG") << "    beginning of main loop" << endl;
			EXTREME_NAMED_LOG("ICPDEBUG") << "    matchNearestNeighbors()" << endl;
            matchNearestNeighbors( tr_template_coords, matched_mol_coords );
			EXTREME_NAMED_LOG("ICPDEBUG") << "    minimizeWeightedDistance()" << endl;
            minimizeWeightedDistance( tr_template_coords, matched_mol_coords,
                                      delta_rot_length, delta_trans_length );
			EXTREME_NAMED_LOG("ICPDEBUG") << "    applyGeomTransformation()" << endl;
            applyGeomTransformation( rotation, translation,
                                     template_coordinates->matValue,
                                     tr_template_coords );
			EXTREME_NAMED_LOG("ICPDEBUG") << "tr_template_coords = " << endl << tr_template_coords << endl;
            error = computeWeightedDistance( tr_template_coords,
                                             matched_mol_coords );
            n_iter++;
			EXTREME_NAMED_LOG("ICPDEBUG") << "end of main loop" << endl;
			EXTREME_NAMED_LOG("ICPDEBUG") << "    iteration = " << n_iter << " / " << max_iter << endl
				<< "    error = " << error << " / " << error_t << endl
				<< "    delta_rot_length = " << delta_rot_length << " / "
				<< angle_t << endl
				<< "    delta_trans_length = " << delta_trans_length << " / "
				<< trans_t << endl
				<< endl;
        }
        while( n_iter < max_iter &&
               error > error_t &&
               delta_rot_length > angle_t &&
               delta_trans_length > trans_t  );

        // keep the best one
        if( error < best_error )
        {
            best_error = error;
            best_rotation << rotation;
            best_translation << translation;
            best_matching << matching;
        }
		EXTREME_NAMED_LOG("ICP+NNet") << "end global iteration number " << i << endl;
    }

    // get best parameters
    error = best_error;
    rotation = best_rotation;
    translation = best_translation;
    matching = best_matching;

    if (memory != "none") {
        // Remember this alignment.
        PLASSERT( find_mol >= 0 );
        mem_error[find_mol] = error;
        mem_rotation[find_mol].resize(rotation.length(), rotation.width());
        mem_rotation[find_mol] << rotation;
        mem_translation[find_mol].resize(translation.length());
        mem_translation[find_mol] << translation;
        mem_matching[find_mol].resize(matching.length());
        mem_matching[find_mol] << matching;
    }

    } else {
        PLASSERT( find_mol >= 0 );
        error = mem_error[find_mol];
        // Copies make the code simpler, though it would be slightly more
        // efficient to use resize and the << operator.
        rotation = mem_rotation[find_mol].copy();
        translation = mem_translation[find_mol].copy();
        matching = mem_matching[find_mol].copy();
    }

    if (find_mol >= 0)
        // Increment counter of alignments for this molecule.
        mem_count[find_mol]++;

    // Update the 'matching_neighbors' variable.
    matching_neighbors->resize(matching.length(), 1);
    for (int i = 0; i < matching.length(); i++)
        matching_neighbors->value[i] = matching[i];

    if( !fast_is_equal( initial_angles_step, 0. ) )
        initial_angles_list.resize( 0, 3 );
}

///////////////////////////
// matchNearestNeighbors //
///////////////////////////
void ChemicalICP::matchNearestNeighbors( const Mat& tr_template_coords,
                                         const Mat& matched_mol_coords )
{
//pout << "matchNearestNeighbors()" << endl;
    Mat mol_coords = mol_coordinates->matValue;
    int n_template_points = tr_template_coords.length();
    int n_mol_points = mol_coords.length();

    if( matching_method == "exhaustive" )
    {
        // bruteforce searche
        for( int i=0 ; i<n_template_points ; i++ )
        {
            Vec t_point = tr_template_coords( i );
//pout << "tr_template_coords(" << i  << ") = " << t_point << endl;
            Vec dists( n_template_points );
            real closest_dist2 = REAL_MAX;
//pout << "closest_dist2 = " << closest_dist2 << endl;

            for( int j=0 ; j<n_mol_points ; j++ )
            {
                // compute distance
                Vec m_point = mol_coords( j );
                real dist2 = powdistance( t_point, m_point, 2 )
                                + feat_distances2(i, j);
/*pout << "m_point = " << m_point << endl;
pout << "powdistance( t_point, m_point, 2 ) = "
    << powdistance( t_point, m_point, 2 ) << endl;
pout << "feat_distances2(" << i << "," << j << ") = " <<
    feat_distances2(i, j) << endl;
pout << "dist2 = " << dist2 << endl;*/

                // keep the smallest
                if( dist2 < closest_dist2 )
                {
//pout << "beep " << endl;
//pout << " matching[" << i << "] = " << j << endl;
                    closest_dist2 = dist2;
                    matching[i] = j;
                }
            }
            matched_mol_coords( i ) << mol_coords( matching[i] );
        }
    }
}

void ChemicalICP::minimizeWeightedDistance( const Mat& tr_template_coords,
                                            const Mat& matched_mol_coords,
                                            real& delta_rot_length,
                                            real& delta_trans_length )
{
    Mat delta_rot( 3, 3 );
    Vec delta_trans( 3 );
    real err = REAL_MAX;

//pout << "before computeWeights()" << endl
//    << tr_template_coords(0) << endl;
    computeWeights( tr_template_coords, matched_mol_coords );
//pout << "after computeWeights() / before transformationFromWeightedMatchedPoints" << endl
//    << tr_template_coords(0) << endl;
    transformationFromWeightedMatchedPoints( tr_template_coords,
                                             matched_mol_coords,
                                             weights,
                                             delta_rot, delta_trans,
                                             err );
/*pout << "after transformationFromWeightedMatchedPoints" << endl
    << tr_template_coords(0) << endl;
pout << "weights = " << weights << endl
     << "delta_rot = " << endl << delta_rot << endl
     << "delta_trans = " << delta_trans << endl;*/

    delta_rot_length = norm( anglesFromRotationMatrix( delta_rot ), 2 );
    delta_trans_length = norm( delta_trans );

    // accumulate transformation ensuring normalization
    Vec angles = anglesFromRotationMatrix( product( rotation, delta_rot ) );
    rotation << rotationMatrixFromAngles( angles );

    // translation = delta_trans + delta_rot * translation
    productAcc( delta_trans, delta_rot, translation );
    translation << delta_trans;
//pout << "rotation = " << endl << rotation << endl
//     << "translation = " << translation << endl;
}

real ChemicalICP::computeWeightedDistance( const Mat& tr_template_coords,
                                           const Mat& matched_mol_coords )
{
//pout << "computeWeightedDistance()" << endl;
    real err = 0;
    int n_points = tr_template_coords.length();
    if( matching_method == "exhaustive" )
    {
        for( int i=0 ; i<n_points ; i++ )
        {
            real diff2 = feat_distances2( i, matching[i] ) +
                powdistance( tr_template_coords(i), matched_mol_coords(i), 2 );
            err += weights[i] * sqrt( diff2 );
/*pout << "i = " << i << endl
    << "tr_template_coords = " << tr_template_coords(i) << endl
    << "matched_mol_coords = " << matched_mol_coords(i) << endl
    << "weights = " <<  weights[i] << endl
    << "delta_err = " << weights[i] * sqrt( diff2 ) << endl
    << endl;*/
        }
    }
    return err;
}

void ChemicalICP::computeWeights( const Mat& tr_template_coords,
                                  const Mat& matched_mol_coords )
{
//pout << "computeWeights()" << endl;
    int n_points = tr_template_coords.length();
//pout << "n_points = " << n_points << endl;

    if( weighting_method == "none" )
    {
//pout << "pouet!" << endl;
        weights.fill( 1. / real( n_points ) );
//pout << "1. / real( n_points ) = " << 1. / real( n_points ) << endl;
//pout << "weights[0] = " << weights[0] << endl;
    }
    else if( weighting_method == "features_sigmoid" )
    {
//pout << "plouf!" << endl;
        real total_weight = 0;
        real mid = weighting_params->value[0];
        real slope = weighting_params->value[1];

        if( matching_method == "exhaustive" )
        {
            for( int i=0 ; i<n_points ; i++ )
            {
                real diff2 = feat_distances2( i, matching[i] );
                weights[i] = sigmoid( slope * (mid - sqrt(diff2)) );
                total_weight += weights[i];
/*pout << "i = " << i << endl
    << "diff2 = " << diff2 << endl
    << "slope = " << slope << endl
    << "mid = " << mid << endl
    << "weights = " <<  weights[i] << endl
    << endl;*/
            }
            weights /= total_weight;
//pout << "total_weight = " << total_weight << endl;
        }
    }
}

void ChemicalICP::saveMatch( const PPath& filename )
{
    PStream file = openFile( filename, PStream::plearn_ascii, "w" );
    file<< "rotation =" << endl
        << rotation << endl
        << "translation =" << endl
        << translation << endl
        << "matching =" << endl
        << matching << endl
        << "error = " << error << endl;
}

// ### Nothing to add here, simply calls build_
void ChemicalICP::build()
{
    inherited::build();
    build_();
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void ChemicalICP::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    varDeepCopyField(mol_feat_indices, copies);
    varDeepCopyField(template_feat_indices, copies);
    varDeepCopyField(matching_neighbors, copies);
    deepCopyField(mol_template, copies);
    deepCopyField(molecule, copies);
    deepCopyField(feature_names, copies);
    varDeepCopyField(weighting_params, copies);
    deepCopyField(initial_angles_list, copies);
    deepCopyField(rotation, copies);
    deepCopyField(translation, copies);
    deepCopyField(matching, copies);
    deepCopyField(weights, copies);
    deepCopyField(used_properties, copies);
    deepCopyField(other_base_properties, copies);

    deepCopyField(mem_aligned_molecules,    copies);
    deepCopyField(mem_error,                copies);
    deepCopyField(mem_rotation,             copies);
    deepCopyField(mem_translation,          copies);
    deepCopyField(mem_matching,             copies);
    deepCopyField(mem_count,                copies);
    
    deepCopyField(used_feat_names, copies);
    deepCopyField(feat_distances2, copies);

    varDeepCopyField(all_mol_features, copies);
    varDeepCopyField(all_template_features, copies);
    varDeepCopyField(all_template_feat_dev, copies);
    varDeepCopyField(mol_coordinates, copies);
    varDeepCopyField(used_mol_features, copies);
    varDeepCopyField(template_coordinates, copies);
    varDeepCopyField(template_geom_dev, copies);
    varDeepCopyField(used_template_features, copies);
    varDeepCopyField(used_template_feat_dev, copies);
}

////////////////////
// declareOptions //
////////////////////
void ChemicalICP::declareOptions(OptionList& ol)
{
    // declareOption(ol, "myoption", &ChemicalICP::myoption, OptionBase::buildoption,
    //               "Help text describing this option");
    // ...

    declareOption(ol, "mol_template", &ChemicalICP::mol_template,
                  OptionBase::buildoption,
                  "The template we try to align on the molecule");

    declareOption(ol, "molecule", &ChemicalICP::molecule,
                  OptionBase::buildoption,
                  "The molecule");

    declareOption(ol, "feature_names", &ChemicalICP::feature_names,
                  OptionBase::buildoption,
                  "Names of features to use during alignment.\n"
                  "Empty TVec means 'use all available features.\n"
                  "Use '[ \"none\" ]' if you don't want to use any feature.\n"
                  );

    declareOption(ol, "weighting_method", &ChemicalICP::weighting_method,
                  OptionBase::buildoption,
                  "Method used to compute the weight of a pair of point."
                  " One of:\n"
                  "    - \"features_sigmoid\": sigmoid of feature distance,\n"
                  "    - \"none\": same weight for each pair.\n"
                  );

    declareOption(ol, "weighting_params", &ChemicalICP::weighting_params,
                  OptionBase::buildoption,
                  "Var containing parameters used during weighting.\n"
                  "Size and meaning depends on value of 'weighting_method'.\n"
                  );

    declareOption(ol, "matching_method", &ChemicalICP::matching_method,
                  OptionBase::buildoption,
                  "Method used to find the nearest neighbors. For the moment,"
                  " only one:\n"
                  "    - \"exhaustive\": exhaustive search (caching feature"
                  " distances).\n"
                  );

    declareOption(ol, "memory", &ChemicalICP::memory, OptionBase::buildoption,
        "The kind of memory allowing ICP to re-use previous alignments:\n"
        "- 'none'   : no memory\n"
        "- 'every-X': the alignment for a given molecule is re-computed only\n"
        "             once every X times it is asked\n"
    );

    declareOption(ol, "initial_angles_step", &ChemicalICP::initial_angles_step,
                  OptionBase::buildoption,
                  "Tries initial rotations every \"initial_angles_step\""
                  " degrees");

    declareOption(ol, "initial_angles_list", &ChemicalICP::initial_angles_list,
                  OptionBase::buildoption,
                  "Explicit list of initial rotations angles");

    declareOption(ol, "max_iter", &ChemicalICP::max_iter,
                  OptionBase::buildoption,
                  "Maximum number of iterations to perform during alignment");

    declareOption(ol, "error_t", &ChemicalICP::error_t,
                  OptionBase::buildoption,
                  "Stop alignment if error falls below this threshold");

    declareOption(ol, "angle_t", &ChemicalICP::angle_t,
                  OptionBase::buildoption,
                  "Stop alignment if angles falls below this threshold");

    declareOption(ol, "trans_t", &ChemicalICP::trans_t,
                  OptionBase::buildoption,
                  "Stop alignment if translation falls below this threshold");

    declareOption(ol, "rotation", &ChemicalICP::rotation,
                  OptionBase::learntoption,
                  "Learned rotation matrix");

    declareOption(ol, "translation", &ChemicalICP::translation,
                  OptionBase::learntoption,
                  "Learned translation vector");

    declareOption(ol, "matching", &ChemicalICP::matching,
                  OptionBase::learntoption,
                  "matching[i] is the index of the molecule point being\n"
                  "the nearest neighbor of template point i.\n");

    declareOption(ol, "weights", &ChemicalICP::weights,
                  OptionBase::learntoption,
                  "Weight of the pair of points (i, matching[i])");

    declareOption(ol, "error", &ChemicalICP::error,
                  OptionBase::learntoption,
                  "Weigted error of the alignment");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

////////////
// build_ //
////////////
void ChemicalICP::build_()
{

#ifdef BOUNDCHECK
    // Variable names for debugging.
    used_mol_features->setName("used_mol_features");
    used_template_features->setName("used_template_features");
    used_template_feat_dev->setName("used_template_feat_dev");
#endif

    if (memory == "none") {
        // A negative value for 'every_x' means it is not used.
        every_x = -1;
    } else if (memory.find("every-") == 0) {
        string x = memory.substr(6);
        every_x = atoi(x.c_str());
        if (every_x <= 0)
            PLERROR("In ChemicalICP::build_ - The X value in 'every-X' (option"
                    " 'memory') must be positive");
    } else
        PLERROR("In ChemicalICP::build_ - Invalid value for option 'memory'");

    if( feature_names.size() > 0 &&
        lowerstring( feature_names[0] ) == "none" )
    {
        // no feature will be used during the alignment nor score computation
        feature_names[0] = "none";
        if( feature_names.size() > 1 )
        {
            PLWARNING("First element of 'feature_names' is 'none', but"
                      " other features are present.\n"
                      "Resizing 'feature_names' to 1.\n");
            feature_names.resize( 1 );
        }
    }

    if( lowerstring( weighting_method ) == "none" || weighting_method == "" )
    {
        weighting_method = "none";
        weighting_params->resize(0, 0);
    }
    else if( lowerstring( weighting_method ) == "features_sigmoid" )
    {
        weighting_method = "features_sigmoid";
        weighting_params->resize(2, 1);
    }
    else
        PLERROR( "ChemicalICP::build_ - weighting_method '%s' is unknown.\n",
                 weighting_method.c_str() );


    if( lowerstring( matching_method ) == "exhaustive" )
        matching_method = "exhaustive";
    else
        PLERROR( "ChemicalICP::build_ - matching_method '%s' is unknown.\n",
                 matching_method.c_str() );

    if( mol_template )
    {
        // make the Var's relative to the template have the right storage
        template_coordinates->defineValueLocation( mol_template->coordinates );
        template_geom_dev->defineValueLocation(
            mol_template->geom_dev.toMat( mol_template->n_points(), 1 ) );
        all_template_features->defineValueLocation( mol_template->features );
        all_template_feat_dev->defineValueLocation( mol_template->feat_dev );

        if( molecule )
        {
            // make as if 'setMolecule' were called
            computeUsedFeatures();
            computeVariables();

            if( matching_method == "exhaustive" )
                cacheFeatureDistances();
        }

        // Resize 'matching_neighbors' so that it has the correct size (i.e.
        // the number of points in the template).
        matching_neighbors->resize(mol_template->n_points(), 1);
    }

    // build VarArray
    used_properties = mol_coordinates
        & template_coordinates & template_geom_dev
        & used_mol_features & used_template_features & used_template_feat_dev;

    other_base_properties = all_mol_features
        & all_template_features & all_template_feat_dev;
//pout << "end build_()" << endl;

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
