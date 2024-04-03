#!/bin/bash

# Define the variable containing the value
startTime="$1"
endTime="$2"
timeStep="$3"
dump="$4"
log="$5"
s0="$6"
Lth="$7"
T="$8"
kV="$9"
lx="${10}"
ly="${11}"
lz="${12}"
id="${13}"
Ncell=$((lx * lx * lx))
NEW_TIME="time -$startTime $endTime $timeStep"

# Define other new values
NEW_DUMP="dump vtk $dump"
NEW_LOG="log $log"
NEW_S0="s0 $s0"
NEW_LTH="Lth $Lth"
NEW_T="T $T"
NEW_KV="kv $kV"
NEW_BOX="box $lx. $ly. $lz. p p p"

# Define the filename with VALUE included
NEW_FILENAME="conf_N${Ncell}_s0${s0}_kV${kV}_T${T}_id${id}"

# Use sed to replace specific values in the original file and save to a new file
sed -e "s/^time .*/$NEW_TIME/" \
    -e "s/^dump .*/$NEW_DUMP/" \
    -e "s/^log .*/$NEW_LOG/" \
    -e "s/^s0 .*/$NEW_S0/" \
    -e "s/^Lth .*/$NEW_LTH/" \
    -e "s/^T .*/$NEW_T/" \
    -e "s/^kv .*/$NEW_KV/" \
    -e "s/^box .*/$NEW_BOX/" \
    conf > "$NEW_FILENAME"

echo "Modified file created: $NEW_FILENAME"