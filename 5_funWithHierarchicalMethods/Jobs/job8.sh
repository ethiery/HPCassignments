#!/usr/bin/env bash
#SBATCH --job-name=TDP5
#SBATCH --output=TDP5.out
#SBATCH --error=TDP5.err
#SBATCH -p mistral
#SBATCH --time=04:00:00
#SBATCH --exclusive
#SBATCH --nodes=8 --ntasks-per-node=1 --cpus-per-task=20

module load intel/mkl/64/11.2/2016.0.0 intel-tbb-oss/intel64/43_20150424oss mpi/openmpi/gcc/1.10.0-tm slurm/14.11.11
cd FunWithBarnes-Hut
make

for N in 200000 400000 800000 1600000
do
	MKL_NUM_THREADS=20 mpiexec -np 8 --bind-to none ./benchDistributed ${N} 6
done
