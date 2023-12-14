#!/bin/bash
#SBATCH --nodes=1
#SBATCH --partition=epyc-64
#SBATCH --cpus-per-task=64
#SBATCH --time=00:05:00
#SBATCH --output=lsh_opt_omp_openmp.out
#SBATCH -Aanakano_429

for VARIABLE in 1 2 4 8 16 32 64
do
	echo $VARIABLE
	./lsh_opt_openmp $VARIABLE
done
