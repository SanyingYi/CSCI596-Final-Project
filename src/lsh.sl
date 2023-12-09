#!/bin/bash
#SBATCH --nodes=1
#SBATCH --ntasks-per-node=2
#SBATCH --time=00:01:59
#SBATCH --output=a.out
#SBATCH -A anakano_429

mpirun -n $SLURM_NTASKS ./lsh

