#!/usr/bin/env bash
#SBATCH --job-name=TDP4
#SBATCH --output=TDP4.out
#SBATCH --error=TDP4.err
#SBATCH -p mistral
#SBATCH --time=04:00:00
#SBATCH --exclusive
#SBATCH --nodes=1 --ntasks-per-node=1 --cpus-per-task=20

module load intel/mkl/64/11.2/2016.0.0 intel-tbb-oss/intel64/43_20150424oss mpi/openmpi/gcc/1.10.0-tm slurm/14.11.11
cd funWithLapack
make

echo '# fineBlockSize tPdgetrf_nopiv' > Output/fineBlockSizeTuning.dat

N=10080

for s in 40 60 80 84 90 96 105 112 120 126 140 160
do
	t=`MKL_NUM_THREADS=20 mpiexec -np 1 --bind-to none ./performance dgetrf_nopiv ${N} ${N} -5 5 42 ${s} ${s} | sed -n 2p`
	echo "${s} ${t}" >> Output/fineBlockSizeTuning.dat
done

