#ifndef RUN_OMP_H
#define RUN_OMP_H

typedef struct DistributedBoard DistributedBoard;

/**
 * Runs a given number of iterations of the game of life.
 * OMP version.
 *
 * @param db            An initialized distributed board
 * @param nbIterations  Number of iterations to run
 */
extern void run_OMP(DistributedBoard *b, int nbIterations);

#endif //RUN_OMP_H
