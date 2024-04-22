#!/bin/bash
s0=5.9000
T=0.00001000
idx=8
dt=0.001
equalTime=10.
Ts=(0.05000000 0.02000000 0.01000000 0.00500000 0.00200000 0.00100000)
equalibrations=(1000 1000 5000 5000 10000 10000)
offsets=(100 100 500 500 1000 1000)
for i in ${!Ts[@]}; do
    T=${Ts[$i]}
    equalTime=${equalibrations[$i]}
    offset=${offsets[$i]}
    bash confGeneration.sh "$equalTime" "$equalTime" "$dt" 500 100 "$s0" 0.02 "$T" 10.0000 8 8 8 "$idx" 3 "$offset" 0.05
    ../../build/tvm -s ${s0} -i ${idx} -n 512 -t "$T" -d "$dt" -e "$equalTime"
done
