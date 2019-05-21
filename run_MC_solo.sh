#!/bin/bash

LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/cappraid01/users/hyjeong/edm_soft/analysis


FILE=/cappraid01/users/hyjeong/edm_soft/mc_results/pC_Elas_200MeV_0.1M_cos.ems
NAME=please
NEVENT=100000

./jediwasa -mode mc -fin file:$FILE -n $NAME -nev $NEVENT -lf please.log -abort
