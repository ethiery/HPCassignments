#!/usr/bin/env bash
#SBATCH --job-name=benchPTHREADS
#SBATCH --output=benchPTHREADS.out
#SBATCH --error=benchPTHREADS.err
#SBATCH --time=04:00:00
#SBATCH --exclusive
#SBATCH --nodes=1 --ntasks-per-node=1 --cpus-per-task=20

module load slurm/14.11.11 build/cmake/3.7.0 compiler/gcc/5.1.0 intel/mkl/64/11.2/2016.0.0 mpi/openmpi/gcc/1.10.0-tm slurm/14.11.11
cmake .
make

rm -f data/benchPTHREADS.dat
echo '# nb_procs 4096 8192 16384 32768' > data/benchPTHREADS.dat

for nbProcs in 1 2 4 8 16
do
    line="${nbProcs} "
    for s in 4096 8192 16384 32768
    do
        time=`NUM_THREADS=${nbProcs} ./main PTHREADS 100 ${s}`
        line="${line} ${time}"
    done
    echo "${line}" >> data/benchPTHREADS.dat
done