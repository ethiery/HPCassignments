#include "runMpiSeq.h"
#include "Board.h"
#include "DistributedBoard.h"

#include <mpi.h>
#include <stdlib.h>

#define SHIFT_LEFT 42
#define SHIFT_RIGHT 43

void run_MPI_seq_sync(DistributedBoard *db, int nbIterations)
{
    int blockSize = (getenv("BLOCK_SIZE") == NULL) ? 4096 : atoi(getenv("BLOCK_SIZE"));
    Board *b = db->b;
    int left = (db->procRank== 0) ? db->nbProcs - 1 : db->procRank - 1;
    int right = (db->procRank == db->nbProcs - 1) ? 0 : db->procRank + 1;

    for (int iter = 0; iter < nbIterations; iter++)
    {
        count_neighbours_block_col(b, 0, b->Nx, blockSize);

        update_state_block_col(b, 0, b->Nx);

        // Send and receive borders / left and right ghost cells
        MPI_Sendrecv(&(b->arr[(b->Nx - 1) * b->ld - 1]), b->ld, MPI_FLOAT, right, SHIFT_RIGHT,
                     &(b->arr[- 1 * b->ld - 1]), b->ld, MPI_FLOAT, left, SHIFT_RIGHT,
                     MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        MPI_Sendrecv(&(b->arr[0 * b->ld - 1]), b->ld, MPI_FLOAT, left, SHIFT_LEFT,
                     &(b->arr[b->Nx * b->ld - 1]), b->ld, MPI_FLOAT, right, SHIFT_LEFT,
                     MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }
}


void run_MPI_seq_async(DistributedBoard *db, int nbIterations)
{
    int blockSize = (getenv("BLOCK_SIZE") == NULL) ? 4096 : atoi(getenv("BLOCK_SIZE"));
    Board *b = db->b;
    int left = (db->procRank== 0) ? db->nbProcs - 1 : db->procRank - 1;
    int right = (db->procRank == db->nbProcs - 1) ? 0 : db->procRank + 1;

    MPI_Request sendLeft, sendRight, recvLeft, recvRight;
    MPI_Send_init(&(b->arr[     0    * b->ld - 1]), b->ld, MPI_FLOAT, left, SHIFT_LEFT, MPI_COMM_WORLD, &sendLeft);
    MPI_Send_init(&(b->arr[(b->Nx-1) * b->ld - 1]), b->ld, MPI_FLOAT, right, SHIFT_RIGHT, MPI_COMM_WORLD, &sendRight);
    MPI_Recv_init(&(b->arr[    -1    * b->ld - 1]), b->ld, MPI_FLOAT, left, SHIFT_RIGHT, MPI_COMM_WORLD, &recvLeft);
    MPI_Recv_init(&(b->arr[b->Nx * b->ld - 1]), b->ld, MPI_FLOAT, right, SHIFT_LEFT, MPI_COMM_WORLD, &recvRight);
    MPI_Request requests[] = { sendLeft, sendRight, recvLeft, recvRight };

    for (int iter = 0; iter < nbIterations; iter++)
    {
        // Update borders
        count_neighbours_col(b, 0);
        count_neighbours_col(b, 1);
        count_neighbours_col(b, b->Nx - 1);
        count_neighbours_col(b, b->Nx - 2);
        update_state_col(b, 0);
        update_state_col(b, b->Nx - 1);

        // Start communication operations (Send and receive borders / left and right ghost cells)
        if (db->procRank & 1)
        {
            MPI_Start(&sendLeft);
            MPI_Start(&recvRight);
            MPI_Start(&sendRight);
            MPI_Start(&recvLeft);
        }
        else
        {
            MPI_Start(&recvRight);
            MPI_Start(&sendLeft);
            MPI_Start(&recvLeft);
            MPI_Start(&sendRight);
        }

        // Cover communication time by updating inner cells
        count_neighbours_block_col(b, 2, b->Nx - 2, blockSize);
        update_state_block_col(b, 1, b->Nx - 1);

        // Make sure communication are over before the next iteration
        MPI_Waitall(4, requests, MPI_STATUSES_IGNORE);
    }
}
