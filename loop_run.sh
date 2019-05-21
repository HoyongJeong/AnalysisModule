#!/bin/bash

BEGIN=49301
END=49534

for i in $(seq ${BEGIN} 1 ${END})
do
#	sbatch ./run_data.sh ${i}
	./run_data.sh ${i} &
	disown %1
done
