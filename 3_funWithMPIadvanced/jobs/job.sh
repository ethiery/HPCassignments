#!/usr/bin/env bash
#SBATCH --job-name=TDP3
#SBATCH --output=TDP3.out
#SBATCH --error=TDP3.err
#SBATCH -p mistral
#SBATCH --time=04:00:00
#SBATCH --exclusive
#SBATCH --nodes=1 --ntasks-per-node=16

module purge
module load intel/mkl/64/11.2/2016.0.0 intel-tbb-oss/intel64/43_20150424oss mpi/openmpi/gcc/1.10.0-tm
cd funWithMPI2
make mrproper
make

for s in 1200 6000 12000 24000
do
	./randomMatrixGenerator ${s} ${s} inputs/A${s}.dat
	./randomMatrixGenerator ${s} ${s} inputs/B${s}.dat

	rm -f outputs/measures${s}.dat
	echo '# nbProcs tSequentialGemm tParallelScatter tParallelGemm tParallelGather' > outputs/measures${s}.dat
	
	tseq=`MKL_NUM_THREADS=1 ./localGemm inputs/A${s}.dat inputs/B${s}.dat`
	echo "1 ${tseq} 0 ${tseq} 0" >> outputs/measures${s}.dat
	
	for c in 4 9 16
	do
  		t=`mpiexec -n ${c} distributedGemm inputs/A${s}.dat inputs/B${s}.dat`
  		echo "${c} ${tseq} ${t}" >> outputs/measures${s}.dat
  	done
done

