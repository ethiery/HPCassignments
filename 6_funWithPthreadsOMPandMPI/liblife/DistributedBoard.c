#include "DistributedBoard.h"
#include "Board.h"
#include "utils.h"

#include <mpi.h>
#include <malloc.h>
#include <assert.h>

void init_distributed_board(DistributedBoard *db, int N, int replicate)
{
    check_MPI();
    MPI_Comm_size(MPI_COMM_WORLD, &db->nbProcs);
    MPI_Comm_rank(MPI_COMM_WORLD, &db->procRank);
    db->leftNgbRank = (db->procRank == 0) ? db->nbProcs - 1 : db->procRank - 1;
    db->rightNgbRank = (db->procRank == db->nbProcs - 1) ? 0 : db->procRank + 1;
    db->N = N;
    db->replicated = replicate;
    db->b = (Board *) malloc(sizeof(Board));

    if (db->replicated)
    {
        init_board(db->b, N, N);
    } else
    {
        assert(N % db->nbProcs == 0);
        init_board(db->b, N/db->nbProcs, N);
    }
}

void set_distributed_row(DistributedBoard *db, int y, int val)
{
    set_row(db->b, y, val);
}

void set_distributed_col(DistributedBoard *db, int x, int val)
{
    if (db->replicated)
    {
        set_col(db->b, x, val);
    } else
    {
        int colOwner = x / db->b->Nx;
        int localX = x - colOwner * db->b->Nx;

        if (db->procRank == colOwner)                                           // Set column on the proc who owns it
            set_col(db->b, localX, val);

        else if ((db->leftNgbRank == colOwner) && (localX == db->b->Nx - 1))    // Update left ghost cells if necessary
            set_col(db->b, -1, val);

        else if ((db->rightNgbRank == colOwner) && (localX == 0))               // Update right ghot cells if necessary
            set_col(db->b, db->b->Nx, val);
    }
}

int compare_distributed_boards(DistributedBoard *db1, DistributedBoard *db2)
{
    check_MPI();

    // Different board size
    if (db1->N != db2->N)
        return 0;

    int Nx = db1->N / db1->nbProcs;

    int s1 = (db1->replicated) ? db1->procRank * Nx : 0;
    int s2 = (db2->replicated) ? db2->procRank * Nx : 0;

    int localMatch = compare_boards_block_col(db1->b, s1, s1 + Nx, db2->b, s2, s2 + Nx);
    int globalMatch;
    MPI_Allreduce(&localMatch, &globalMatch, 1, MPI_INT, MPI_LAND, MPI_COMM_WORLD);

    return globalMatch;
}


void free_distributed_board(DistributedBoard *db)
{
    free_board(db->b);
    free(db->b);
}
