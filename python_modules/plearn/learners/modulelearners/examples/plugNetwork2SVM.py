import sys
from plearn.learners.modulelearners  import *
from plearn.learners.SVM import *

if __name__ == '__main__':

    if len(sys.argv) = 1:
       ports_list = ['split.out1', 'split.out2',  'rbm_3.hidden.state', 'rbm_12.hidden.state']
    else:
       ports_list = sys.argv[1:]

    learner_filename = "learner.psave"
    data_filename = "samples.amat"
    dataTrain_filename = data_filename
    dataTest_filename = data_filename
    dataValid_filename = data_filename

    if os.path.isfile(learner_filename) == False:
       raise TypeError, "ERROR : Learner file cannot be find\n\tCould not find file "+learner_filename
    learner = loadModuleLearner(learner_filename)
    learner_nickname = 'DBN-2-2-1_'+"_".join(ports_list).replace(".","")

    result_dir = os.path.dirname(learner_filename)
    output_filename = result_dir+'/SVM_results_'+"_"+learner_nickname+"-"+os.path.basename(dataTrain_filename).replace(".vmat","").replace(".amat","")


#               #
#   MAIN PART   #
#               #


    new_learner = plug2output( learner, ports_list)

    
  
    for typeDataSet in ['Train','Valid','Test']:
        data_filename = globals()['data'+typeDataSet+'_filename']
        if os.path.isfile(data_filename) == False:
           print "ERROR : Data file cannot be find\n\tCould not find file "+data_filename
           sys.exit(0)
        print "CONVERSION "+data_filename
	dataSet = pl.AutoVMatrix( filename = data_filename )
        globals()[typeDataSet+'_outputs'], globals()[typeDataSet+'_targets'] = computeOutputsTargets( new_learner, dataSet)
	#
	# Normalizing the data (/!\ compute statistics on the training data and assumes it comes first)
	#
        if typeDataSet == 'Train':
           mean, std = normalize(globals()[typeDataSet+'_outputs'],None,None)
	else:
	   normalize(globals()[typeDataSet+'_outputs'],mean,std)

    E=SVM()
    
    print "Writing results in "+output_filename
    if os.path.isfile(output_filename):
       print "WARNING : output "+output_filename+" already exists"
       FID = open(output_filename, 'a')
       abspath = os.path.realpath(learner_filename)
       FID.write('LEARNER.: '+abspath+'\n')
       for i in range(3):
           abspath = os.path.dirname(abspath)
       global_results = abspath+'/global_stats.pmat'
       if os.path.isfile(global_results):
          os.system("echo   baseline test error rate : `plearn vmat cat "+global_results+" | tail -1 | awk '{print $NF}'` \%   >> "+output_filename )
       else:
          print "WARNING : could not find global_stats.pmat\n\t( "+abspath+"/global_stats.pmat )"
       FID.write('Train...: '+os.path.realpath(dataTrain_filename)+'\n')
       FID.write('Valid...: '+os.path.realpath(dataValid_filename)+'\n')
       FID.write('Test....: '+os.path.realpath(dataTest_filename)+'\n')
    else:
       FID = open(output_filename, 'w')
    FID.write('--------\n')

    E.save_filename = output_filename
    E.train_and_tune( 'LINEAR' ,     [[Train_outputs,Train_targets], [Valid_outputs,Valid_targets], [Test_outputs,Test_targets]])
    FID = open(output_filename, 'a')
    FID.write("Tried parameters : "+str(E.tried_parameters)+'\n')
    FID.write('BEST ERROR RATE: '+str(E.valid_error_rate)+' (valid) - '+str(E.error_rate)+' (test) for '+str(E.best_parameters)+'\n')
    FID.close()
    E.train_and_tune( 'RBF' ,     [[Train_outputs,Train_targets], [Valid_outputs,Valid_targets], [Test_outputs,Test_targets]])
    FID = open(output_filename, 'a')
    FID.write("Tried parameters : "+str(E.tried_parameters)+'\n')
    FID.write('BEST ERROR RATE: '+str(E.valid_error_rate)+' (valid) - '+str(E.error_rate)+' (test) for '+str(E.best_parameters)+'\n')
    FID.close()
    E.train_and_tune( 'RBF' ,     [[Train_outputs,Train_targets], [Valid_outputs,Valid_targets], [Test_outputs,Test_targets]])
    FID = open(output_filename, 'a')
    FID.write("Tried parameters : "+str(E.tried_parameters)+'\n')
    FID.write('BEST ERROR RATE: '+str(E.valid_error_rate)+' (valid) - '+str(E.error_rate)+' (test) for '+str(E.best_parameters)+'\n')
    FID.close()
    print "Results written in "+output_filename