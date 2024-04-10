In order to submit batch jobs, I've make a few modifacations
Make sure to spefify the path to conf in the tvm.cpp and also in the script that generates conf
The workDir in run.h is for saving data.
When using bash scripts to generate conf, make sure the number of digits matches the vm.cpp and get the right path to conf.
Make sure the tell tvm.cpp the path to the sample.topo.

I add the choice to save cell centers with log spacing and different offset. A few things to keep in mind:
    1. The log space is on time steps instead of simulation time.
    2. Offset spaing is all time steps. The waiting time is offset times dt.
    3. Offsets and log spaced saving all starts from positive time.