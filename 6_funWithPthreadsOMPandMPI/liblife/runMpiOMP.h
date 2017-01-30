#ifndef RUN_MPI_OMP_H
#define RUN_MPI_OMP_H

typedef struct DistributedBoard DistributedBoard;

/**
 * Runs a given number of iterations of the game of life.
 * MPI + OMP synchroneous version.
 *
 * @param db An initialized distributed board
 * @param nbIterations Number of iterations to run
 */
extern void run_MPI_OMP_sync(DistributedBoard *db, int nbIterations);

/**
 * Runs a given number of iterations of the game of life.
 * MPI + OMP asynchroneous version.
 *
 * @param db An initialized distributed board
 * @param nbIterations Number of iterations to run
 */
extern void run_MPI_OMP_async(DistributedBoard *db, int nbIterations);

#endif //RUN_MPI_OMP_H
