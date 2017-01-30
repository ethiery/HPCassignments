#!/usr/bin/env bash

runners="SEQ OMP PTHREADS MPI_SEQ_SYNC MPI_SEQ_ASYNC"

line="# nb_procs"
for runner in ${runners}
do
    for size in 4096 8192 16384 32768
    do
        line="${line} ${runner}_${size}"
    done
done

echo "${line}" > data/benchALL.dat

for nbProcs in 1 2 4 8 16
do
    line="${nbProcs} "
    for runner in ${runners}
    do
        subline=`cat data/bench${runner}.dat | grep "^${nbProcs} " | cut -d " " -f 2-`
        line="${line} ${subline}"
    done
    echo "${line}" >> data/benchALL.dat
done