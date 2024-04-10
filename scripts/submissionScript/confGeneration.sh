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
noffset="${14}"
spacingoffsets="${15}"
logspacing="${16}"

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
NEW_OFFSETS="offsets $noffset $spacingoffsets $logspacing"

# Define the filename with VALUE included
NEW_FILENAME="conf_N${Ncell}_s0${s0}_kV${kV}_T${T}_id${id}"
{
    echo "$NEW_TIME" 
    echo "$NEW_DUMP" 
    echo "$NEW_LOG"
    echo "$NEW_S0" 
    echo "$NEW_LTH" 
    echo "$NEW_T" 
    echo "$NEW_KV" 
    echo "$NEW_BOX" 
    echo "$NEW_OFFSETS" 
} > $NEW_FILENAME