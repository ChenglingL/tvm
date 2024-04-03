#!/bin/bash
s0=5.9000
T=0.00001000
idx=8
bash confGeneration.sh 10 10 0.0005 1 1 "$s0" 0.02 "$T" 10.0000 8 8 8 "$idx"
../../build/tvm -s ${s0} -i ${idx} -n 512 -t "$T"