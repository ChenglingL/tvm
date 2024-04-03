In order to submit batch jobs, I've make a few modifacations
Make sure to spefify the path to conf in the tvm.cpp and also in the script that generates conf
The workDir in run.h is for saving data.
When using bash scripts to generate conf, make sure the number of digits matches the vm.cpp and get the right path to conf.
Make sure the tell tvm.cpp the path to the sample.topo.