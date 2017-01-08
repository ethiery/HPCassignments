#!/usr/bin/env bash
#SBATCH --job-name=TDP4
#SBATCH --output=TDP4.out
#SBATCH --error=TDP4.err
#SBATCH -p mistral
#SBATCH --time=04:00:00
#SBATCH --exclusive
#SBATCH --nodes=9 --ntasks-per-node=1 --cpus-per-task=20

module purge
module load intel/mkl/64/11.2/2016.0.0 intel-tbb-oss/intel64/43_20150424oss mpi/openmpi/gcc/1.10.0-tm slurm/14.11.11
cd funWithLapack
make mrproper
make

for s in 1200 6000 12000 24000
do
	./randomMatrixGenerator ${s} ${s} inputs/A${s}.dat
	./randomMatrixGenerator ${s} ${s} inputs/B${s}.dat

	rm -f outputs/measuresInter${s}.dat
	echo '# nbProcs tSequentialGemm tParallelScatter tParallelGemm tParallelGather' > outputs/measuresInter${s}.dat
	
	tseq=`MKL_NUM_THREADS=1 ./localGemm inputs/A${s}.dat inputs/B${s}.dat`
	echo "1 ${tseq} 0 ${tseq} 0" >> outputs/measuresInter${s}.dat
	
	for c in 4 9
	do
  		t=`mpiexec -n ${c} distributedGemm inputs/A${s}.dat inputs/B${s}.dat`
  		echo "${c} ${tseq} ${t}" >> outputs/measuresInter${s}.dat
  	done
done

