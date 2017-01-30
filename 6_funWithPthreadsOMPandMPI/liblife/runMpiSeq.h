#ifndef RUN_MPI_H
#define RUN_MPI_H

typedef struct DistributedBoard DistributedBoard;

/**
 * Runs a given number of iterations of the game of life.
 * MPI + sequential synchroneous version.
 *
 * @param db An initialized distributed board
 * @param nbIterations Number of iterations to run
 */
extern void run_MPI_seq_sync(DistributedBoard *db, int nbIterations);

/**
 * Runs a given number of iterations of the game of life.
 * MPI + sequential asynchroneous version.
 *
 * @param db An initialized distributed board
 * @param nbIterations Number of iterations to run
 */
extern void run_MPI_seq_async(DistributedBoard *db, int nbIterations);


#endif //RUN_MPI_H
