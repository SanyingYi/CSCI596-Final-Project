#!/bin/bash
#SBATCH --nodes=1
#SBATCH --ntasks-per-node=1
#SBATCH --partition=gpu
#SBATCH --gres=gpu:a40:1
#SBATCH --time=00:05:59
#SBATCH --output=lsh_opt_omp_teams.out
#SBATCH -Aanakano_429

./lsh_opt_omp_teams
