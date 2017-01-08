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
rm -f generated/time1024.dat
touch generated/time1024.dat
echo '# nbProcs tSeq tPar' >> generated/time1024.dat
tseq=`./simulator -i inputs/1024.dat -s -r 0.01 -n 10000`
echo "1 ${tseq} ${tseq}" >> generated/time1024.dat
for i in 2 4 8 16 32 64
do
  t=`mpiexec -n $i ./simulator -i inputs/1024.dat -r 0.1 -n 10000 | head -n 1`
  echo "${i} ${tseq} ${t}" >> generated/time1024.dat
done
