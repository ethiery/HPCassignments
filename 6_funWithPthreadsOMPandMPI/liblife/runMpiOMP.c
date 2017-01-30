#include "runMpiOMP.h"
#include "DistributedBoard.h"
#include "Board.h"

#include <omp.h>
#include <mpi.h>
#include <stdlib.h>

#define SHIFT_LEFT 42
#define SHIFT_RIGHT 43

void run_MPI_OMP_sync(DistributedBoard *db, int nbIterations)
{
    int blockSize = (getenv("BLOCK_SIZE") == NULL) ? 4096 : atoi(getenv("BLOCK_SIZE"));
    int nbThreads = (getenv("NUM_THREADS") == NULL) ? 1: atoi(getenv("NUM_THREADS"));
    omp_set_num_threads(nbThreads);
    Board *b = db->b;
    int left = (db->procRank== 0) ? db->nbProcs - 1 : db->procRank - 1;
    int right = (db->procRank == db->nbProcs - 1) ? 0 : db->procRank + 1;

    #pragma omp parallel
    for (int iter = 0; iter < nbIterations; iter++)
    {
        count_neighbours_block_col(b, 0, b->Nx, blockSize); // Count neighbours

        update_state_block_col(b, 0, b->Nx);                // Contains omp for. Takes care of top + bottom ghost cells

        // Send and receive borders / left and right ghost cells
        #pragma omp single
        MPI_Sendrecv(&(b->arr[(b->Nx - 1) * b->ld - 1]), b->ld, MPI_FLOAT, right, SHIFT_RIGHT,
                     &(b->arr[- 1 * b->ld - 1]), b->ld, MPI_FLOAT, left, SHIFT_RIGHT,
                     MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        #pragma omp single
        MPI_Sendrecv(&(b->arr[0 * b->ld - 1]), b->ld, MPI_FLOAT, left, SHIFT_LEFT,
                     &(b->arr[b->Nx * b->ld - 1]), b->ld, MPI_FLOAT, right, SHIFT_LEFT,
                     MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        // Make sure communication are over before the next iteration
        #pragma omp barrier
    }
}