#!/bin/bash
#SBATCH --nodes=1
#SBATCH --ntasks-per-node=8
#SBATCH --time=00:04:59
#SBATCH --output=a18.out
#SBATCH -A anakano_429

mpirun -n $SLURM_NTASKS ./lsh_mpi

