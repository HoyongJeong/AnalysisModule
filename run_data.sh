#!/bin/bash

# Env
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/users/hyjeong/works/wasa_analysis/0_RawAnalysis
#source /users/hyjeong/SetWASAEnv5.sh

# Run number
RUNNO=${1}
FILEIN=/users/hyjeong/storage02/data/wasa/201808_PDBR_raw/run_${RUNNO}
ANANAME=ana_${RUNNO}

#cd /users/hyjeong/works/wasa_analysis/0_RawAnalysis

if [ -f ${FILEIN} ];  then
	./jediwasa -fin cluster:$FILEIN -n ${ANANAME} -tree 1 -binsize 0.2 -abort -local kTRUE -v 1
else
	echo "Run ${RUNNO} does not exist"
fi
