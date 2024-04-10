#!/bin/bash
s0=5.9000
T=0.00001000
idx=8
bash confGeneration.sh 0 100 0.001 1 1 "$s0" 0.02 "$T" 10.0000 8 8 8 "$idx" 3 10000 0.1
../../build/tvm -s ${s0} -i ${idx} -n 512 -t "$T"