#include <stdlib.h>

#include <DistributedBoard.h>
#include <runFuncs.h>
#include <utils.h>

void setup(DistributedBoard *db, int N, int replicate)
{
    init_distributed_board(db, N, replicate);
    set_distributed_row(db, N/2, 1);
    set_distributed_col(db, N/2, 1);
}

int main(int argc, char** argv)
{
    if (argc != 3)
    {
        fflush(stdout);
        printf("Usage: NUM_THREADS=X mpiexec -n Y %s nb_iterations board_size\n"
                       "with board_size a multiple of X * Y\n", argv[0]);
        return EXIT_FAILURE;
    }

    int nbIters = atoi(argv[1]);
    int N = atoi(argv[2]);


    DistributedBoard dbSeq;
    setup(&dbSeq, N, 1);
    run_seq(&dbSeq, nbIters);

    DistributedBoard db;
    for (int i = 0; i < nbRunFuncs; i++)
    {
        setup(&db, N, replicationParams[i]);
        runFuncs[i](&db, nbIters);
        fprintf_once(stdout, "Comparing SEQ and %s results: ", runFuncNames[i]);
        fprintf_once(stdout, compare_distributed_boards(&dbSeq, &db) ? "identical\n" : "different\n");
        free_distributed_board(&db);
    }

    free_distributed_board(&dbSeq);

    return EXIT_SUCCESS;
}