#include <stdlib.h>
#include "runSeq.h"
#include "Board.h"
#include "DistributedBoard.h"

void run_seq(DistributedBoard *db, int nbIterations)
{
    int blockSize = (getenv("BLOCK_SIZE") == NULL) ? 4096 : atoi(getenv("BLOCK_SIZE"));
    Board *b = db->b;

    for (int iter = 0; iter < nbIterations; iter++)
    {
        count_neighbours_block_col(b, 0, b->Nx, blockSize);

        update_state_block_col(b, 0, b->Nx);                // this takes care of top and bottom ghost cells
        copy_state_col(b, -1, b->Nx - 1);                   // this takes care of left and right ghost cells
        copy_state_col(b, b->Nx, 0);
    }
}