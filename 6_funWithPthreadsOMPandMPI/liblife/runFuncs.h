#ifndef RUNFUNCS_H
#define RUNFUNCS_H

#include "runSeq.h"
#include "runOMP.h"
#include "runMpiOMP.h"
#include "runPthreads.h"
#include "runMpiSeq.h"

typedef struct DistributedBoard DistributedBoard;
typedef void (*runFunc)(DistributedBoard *, int);

const int nbRunFuncs = 6;
const char runFuncNames[][255] = { "SEQ",   "OMP",   "PTHREADS",   "MPI_SEQ_SYNC",   "MPI_SEQ_ASYNC",   "MPI_OMP_SYNC"};
const runFunc runFuncs[] = {     run_seq, run_OMP, run_pthreads, run_MPI_seq_sync, run_MPI_seq_async, run_MPI_OMP_sync};
const int replicationParams[] = {      1,       1,            1,                 0,                0,                0};

#endif //RUNFUNCS_H
