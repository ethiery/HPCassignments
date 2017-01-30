#!/usr/bin/env bash
#SBATCH --job-name=cacheOptimization
#SBATCH --output=cacheOptimization.out
#SBATCH --error=cacheOptimization.err
#SBATCH --time=04:00:00
#SBATCH --exclusive
#SBATCH --nodes=1 --ntasks-per-node=1 --cpus-per-task=1

module load slurm/14.11.11 build/cmake/3.7.0 compiler/gcc/5.1.0 intel/mkl/64/11.2/2016.0.0 mpi/openmpi/gcc/1.10.0-tm slurm/14.11.11
cmake .
make

rm -f data/cacheOptimization.dat
echo '# block_size 512 512_noblock 1024 1024_noblock 2048 2048_noblock 4096 4096_noblock 8192 8192_noblock 16384 16384_noblock 32768 32768_noblock' > data/cacheOptimization.dat

declare -a naiveTime

for s in 512 1024 2048 4096 8192 16384 32768
do
    naiveTime[${s}]=`BLOCK_SIZE=${s} NUM_THREADS=1 ./main SEQ 100 ${s}`
done

for bs in 256 512 1024 2048 4096 8192 16384 32768
do
    line="${bs} "
    for s in 512 1024 2048 4096 8192 16384 32768
    do
        blockTime=`BLOCK_SIZE=${bs} NUM_THREADS=1 ./main SEQ 100 ${s}`
        line="${line} ${blockTime} ${naiveTime[${s}]}"
    done
    echo "${line}" >> data/cacheOptimization.dat
done