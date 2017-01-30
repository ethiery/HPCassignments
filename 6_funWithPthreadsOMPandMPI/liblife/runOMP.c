#include "runOMP.h"
#include "Board.h"
#include "DistributedBoard.h"

#include <omp.h>
#include <stdlib.h>

void run_OMP(DistributedBoard *db, int nbIterations)
{
    int blockSize = (getenv("BLOCK_SIZE") == NULL) ? 4096 : atoi(getenv("BLOCK_SIZE"));
    int nbThreads = (getenv("NUM_THREADS") == NULL) ? 1: atoi(getenv("NUM_THREADS"));
    omp_set_num_threads(nbThreads);
    Board *b = db->b;

    #pragma omp parallel
    for (int iter = 0; iter < nbIterations; iter++)
    {
        count_neighbours_block_col(b, 0, b->Nx, blockSize);// Contains omp for.

        update_state_block_col(b, 0, b->Nx);               // Contains omp for. Takes care of top and bottom ghost cells
        #pragma omp single
        copy_state_col(b, -1, b->Nx - 1);                  // Takes car of left ghost cells
        #pragma omp single
        copy_state_col(b, b->Nx, 0);                       // Takes car of right ghost cells

        #pragma omp barrier
    }
}