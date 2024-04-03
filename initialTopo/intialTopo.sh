#!/bin/bash
idces=(0 1 2 3 4 5 6 7 8 9)
for idx in ${idces[@]}; do
    python ../scripts/tvm/main.py 8 8 8 ${idx}
 done
