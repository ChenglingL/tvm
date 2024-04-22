#!/bin/bash
s0=5.9000
T=0.00010000
idx=8
dt=0.001
equalTime=0.
bash confGeneration.sh "$equalTime" 100 "$dt" 1 2 "$s0" 0.02 "$T" 10.0000 8 8 8 "$idx" 3 1000 0.1
../../build/tvm -s ${s0} -i ${idx} -n 512 -t "$T" -d "$dt" -e "$equalTime"