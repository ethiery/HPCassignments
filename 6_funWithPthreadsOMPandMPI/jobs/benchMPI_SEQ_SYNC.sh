#!/usr/bin/env bash
#SBATCH --job-name=benchMPI_SEQ_SYNC
#SBATCH --output=benchMPI_SEQ_SYNC.out
#SBATCH --error=benchMPI_SEQ_SYNC.err
#SBATCH --time=04:00:00
#SBATCH --exclusive
#SBATCH --nodes=8 --ntasks-per-node=1 --cpus-per-task=1

module load slurm/14.11.11 build/cmake/3.7.0 compiler/gcc/5.1.0 intel/mkl/64/11.2/2016.0.0 mpi/openmpi/gcc/1.10.0-tm slurm/14.11.11
cmake .
make

rm -f data/benchMPI_SEQ_SYNC.dat
echo '# nb_processes 4096 8192 16384 32768' > data/benchMPI_SEQ_SYNC.dat

for nbProcesses in 1 2 4 8
do
    line="${nbProcesses} "
    for s in 4096 8192 16384 32768
    do
        time=`mpiexec -np ${nbProcesses} --bind-to none ./main MPI_SEQ_SYNC 100 ${s}`
        line="${line} ${time}"
    done
    echo "${line}" >> data/benchMPI_SEQ_SYNC.dat
done