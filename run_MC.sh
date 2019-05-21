#!/bin/bash

#energy=(140 150 160 170 172 180 190 200 202 210 220 230 233 240 250 260)
energy=(190 200 210 220 233 250)

#./jediwasa -mode mc -fin file:/data0/wasa/trees/mc/ems/protons_0_400MeV_theta_all_quench_deuteron_25e6_3.ems -n /data0/wasa/trees/mc/ems/protons_0_400MeV_theta_all_quench_deuteron_25e6_3 -lf log.log -abort
#./jediwasa -mode mc -fin file:/data0/wasa/trees/mc/ems/deuterons_0_400MeV_theta_FTHwrap5_1e4.ems -n /data0/wasa/trees/mc/deuterons_0_400MeV_theta_FTHwrap5_1e4 -lf log.log -abort

for i in "${energy[@]}"
do
	#./jediwasa -mode mc -fin file:/home/hyjeong/storage06/MC_results/pC_elastic_${energy}MeV_1.ems -n pC_elastic_${energy}MeV_1 -nev 1000000 -lf pC_elastic_${energy}MeV_1.log -abort
	./jediwasa -mode mc -fin file:/cappraid01/users/hyjeong/edm_soft/mc_results/pC_Elas_${i}MeV_10M_cos.ems -n pC_Elas_${i}MeV_10M_cos -nev 10000000 -lf pC_Elas_${i}MeV_10M_cos.log -abort
done
