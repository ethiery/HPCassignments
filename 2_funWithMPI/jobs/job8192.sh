#!/usr/bin/env bash
#SBATCH --job-name=TDP2_512
#SBATCH --output=/tmp/512.out
#SBATCH --error=/tmp/.512.err
#SBATCH -p mistral
#SBATCH --time=00:30:00
#SBATCH --exclusive
#SBATCH --nodes=4 --ntasks-per-node=16

module load mpi/openmpi/gcc/1.10.1-tm
cd /home/prcd2016-thiery/funWithMPI
make
rm -f generated/time8192.dat
touch generated/time8192.dat
echo '# nbProcs tPar' >> generated/time8192.dat
for i in 16 32 64
do
  t=`mpiexec -n $i ./simulator -i inputs/8192.dat -r 0.1 -n 10000 | head -n 1`
  echo "${i} ${t}" >> generated/time8192.dat
done
