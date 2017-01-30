#ifndef RUN_REF_H
#define RUN_REF_H

typedef struct DistributedBoard DistributedBoard;

/**
 * Runs a given number of iterations of the game of life.
 * Reference sequential version.
 *
 * @param db           An initialized distributed board
 * @param nbIterations The number of iterations to run
 */
extern void run_seq(DistributedBoard *db, int nbIterations);

#endif //RUN_REF_H