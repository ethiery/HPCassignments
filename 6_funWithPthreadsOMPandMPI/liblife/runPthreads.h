#ifndef RUN_PTHREADS_H
#define RUN_PTHREADS_H

typedef struct DistributedBoard DistributedBoard;

/**
 * Runs a given number of iterations of the game of life.
 * Pthreads version.
 *
 * @param db            An initialized distributed board
 * @param nbIterations  Number of iterations to run
 */
extern void run_pthreads(DistributedBoard *db, int nbIterations);


#endif //RUN_PTHREADS_H
