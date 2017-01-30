#!/usr/bin/env bash
#SBATCH --job-name=benchMPI_OMP_SYNC
#SBATCH --output=benchMPI_OMP_SYNC.out
#SBATCH --error=benchMPI_OMP_SYNC.err
#SBATCH --time=04:00:00
#SBATCH --exclusive
#SBATCH --nodes=8 --ntasks-per-node=1 --cpus-per-task=8

module load slurm/14.11.11 build/cmake/3.7.0 compiler/gcc/5.1.0 intel/mkl/64/11.2/2016.0.0 mpi/openmpi/gcc/1.10.0-tm
cmake .
make

rm -f data/benchMPI_OMP_SYNC.dat
echo '# nb_processes 8192_local 16384_local 32768_local 8192 16384 32768' > data/benchMPI_OMP_SYNC.dat

declare -a localTime

localLine=""
for s in 8192 16384 32768
do
    t=`./main SEQ 100 ${s}`
    localLine="${localLine} ${t}"
done
echo "1 ${localLine} ${localLine}" >> data/benchMPI_OMP_SYNC.dat


for nbProcesses in 1 2 4 8
do
    nbThreads=$((8 * ${nbProcesses}))
    line="${nbThreads} ${localLine}"
    for s in 8192 16384 32768
    do
        time=`NUM_THREADS=8 mpiexec -np ${nbProcesses} --bind-to socket ./main MPI_OMP_SYNC 100 ${s}`
        line="${line} ${time}"
    done
    echo "${line}" >> data/benchMPI_OMP_SYNC.dat
done