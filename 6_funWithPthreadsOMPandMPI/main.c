#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include <DistributedBoard.h>
#include <runFuncs.h>
#include <Timer.h>
#include <utils.h>

int main(int argc, char** argv)
{
    if (argc != 4)
    {
        printf("Usage: NUM_THREADS=X %s {SEQ|OMP|PTHREADS|MPI_SEQ_ASYNC|MPI_SEQ_SYNC|MPI_OMP_SYNC} nb_iterations board_size\n"
                       "Note: \n"
                       "- The number of thread passed as an environment variable is only taken into account OMP, PTHREADS and MPI_OMP_SYNC\n"
                       "- board_size must be a multiple of the number of MPI processes for MPI versions"
                       "- board_size must be a multiple of the number of threads for PTHREADS\n", argv[0]);
        return EXIT_FAILURE;
    }

    int nbIters = atoi(argv[2]);
    int N = atoi(argv[3]);
    runFunc run = run_seq;
    int replicate = 1;

    for (int i = 0; i < nbRunFuncs; i++)
        if (strncmp(argv[1], runFuncNames[i], 255) == 0)
        {
            run = runFuncs[i];
            replicate = replicationParams[i];
            break;
        }

    // The initial state of the board is a centered cross
    DistributedBoard db;
    init_distributed_board(&db, N, replicate);
    set_distributed_row(&db, N/2, 1);
    set_distributed_col(&db, N/2, 1);

    Timer *t = start_timer();
    run(&db, nbIters);
    fprintf_once(stdout, "%e\n", stop_timer(t));

    free_distributed_board(&db);

    return EXIT_SUCCESS;
}