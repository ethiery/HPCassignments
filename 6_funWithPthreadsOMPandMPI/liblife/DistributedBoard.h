#ifndef DISTRIBUTED_BOARD_H
#define DISTRIBUTED_BOARD_H

typedef struct Board Board;

/**
 * A distributed board can be either:
 * - a local vertical stripe of a global board that is split between processes
 * - a global board replicated across all processes
 *
 * This allows to compare results between a non distributed implementation and
 * a distributed implementation by running the former on a replicated db,
 * and the latter on a split db initialized with the same values.
 */
typedef struct DistributedBoard
{
    int N;              // Size of the global board (N x N)
    int nbProcs;        // Number of processes the global board is distributed across
    int procRank;       // Rank of the current process
    int leftNgbRank;    // Rank of the left neighbour process
    int rightNgbRank;   // Rank of the right neighbour process

    int replicated; // 1 if the board is replicated, 0 if it is split
    Board *b;       // Local stripe or replica

} DistributedBoard;

/**
 * Initialize a handle over a distributed board.
 *
 * When initializing a split distributed board, its size must be a multiple of the
 * number of MPI processes executing this function.
 *
 * @param db            Allocated but uninitialized DistributedBoard
 * @param N             Size of the global board
 * @param replicate     Set to 1 to replicate the global board across processes, 0 to split it between them
 */
extern void init_distributed_board(DistributedBoard *db, int N, int replicate);

/**
 * Set the state of a column of cells
 *
 * @param b         Pointer on an initialized distributed board
 * @param x         X-coordinate of the column
 * @param val       Value to set
 */
extern void set_distributed_col(DistributedBoard *db, int x, int val);

/**
 * Set the state of a row of cells
 *
 * @param b         Pointer on an initialized distributed board
 * @param y         Y-coordinate of the row
 * @param val       Value to set
 */
extern void set_distributed_row(DistributedBoard *db, int y, int val);

/**
 * Compare two distributed boards
 *
 * @param db1        An initialized distributed board
 * @param db2        An initialized distributed board
 * @return 1 if the boards are identical, 0 else
 */
extern int compare_distributed_boards(DistributedBoard *db1, DistributedBoard *db2);

/**
 * Release the resources associated with a distributed board.
 *
 * @param b An initialized distributed board
 */
extern void free_distributed_board(DistributedBoard *db);

#endif //DISTRIBUTED_BOARD_H
