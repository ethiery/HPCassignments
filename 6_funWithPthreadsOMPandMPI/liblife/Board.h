#ifndef BOARD_H
#define BOARD_H

#include <math.h>
#include <stdio.h>

/* Data structure */

/**
 * A board is an array of Nx x Ny inner cells framed by a layer of ghost cells, used for communication with other MPI nodes.
 * This means that -1 and Nx (resp. Ny) are valid indices for x coordinates (resp y coordinates)
 */
typedef struct Board
{
    int Nx, Ny;     // width and height of the board
    float *nbNgbs;  // nb of neighbours of each cell (ghost cells excluded), Nx x Ny Col major array

    int ld;         // Ny + 2
    float *arr;     // state of each cell (ghost cell included) ld x (Ny+2) col major array,
                    // index 0,0 actually points on the second row of the second column (top left inner cell)
} Board;


/* Parameters */


/**
 * State change rules definition.
 * STATE_CHANGES[currentState * 9 + nbNeighbours] gives the next state of a cell given its current state
 * and its current number of neighbours
 */
static const float STATE_CHANGES[] = { 0, 0, 0, 1, 0, 0, 0, 0, 0,
                                       0, 0, 1, 1, 0, 0, 0, 0, 0 };


/* Public methods */


/**
 * Initialize an empty board of specified size (not counting ghost cells)
 *
 * @param b         Pointer on an uninitialized board
 * @param Nx        Horizontal size of the board
 * @param Ny        Vertical size of the board
 */
extern void init_board(Board *b, int Nx, int Ny);


/**
 * Set the state of a column of cells (ghost cells included)
 * @param b         Pointer on an initialized board
 * @param x         X-coordinate of the column
 * @param val       Value to set
 */
extern void set_col(Board *b, int x, int val);


/**
 * Set the state of a row of cells (ghost cells included)
 * @param b         Pointer on an initialized board
 * @param y         Y-coordinate of the row
 * @param val       Value to set
 */
extern void set_row(Board *b, int y, int val);


/**
 * Update the neighbours count of a column of cells
 *
 * @param b An initialized board
 * @param x X-coordinate of a column
 */
extern void count_neighbours_col(Board *b, int x);


/**
 * Update the state of a column of cells (ghost cells included)
 *
 * @param b An initialized board
 * @param x X-coordinate of a column
 */
extern void update_state_col(Board *b, int x);


/**
 * Copy the state a source column in a destination column (ghost cells included)
 * @param b    (Initialized) board to update
 * @param xDst X-coordinate of the destination column
 * @param xSrc X-coordinate of the source column
 */
extern void copy_state_col(Board *b, int xDst, int xSrc);


/**
 * Update the neighbours count of a block column of cells.
 * To optimize cache access, use vertical blocking too.
 *
 * @param b             An initialized board
 * @param xStart        Starting X-coordinate of the block column
 * @param xStop         Stopping X-coordinate of the block column
 * @param blockSize     Vertical blocking size used for cache optimization
 */
extern void count_neighbours_block_col(Board *b, int xStart, int xStop, int blockSize);


/**
 * Update the state of a block column of cells (ghost cells included)
 *
 * @param b             An initialized board
 * @param xStart        Starting X-coordinate of the block column
 * @param xStop         Stopping X-coordinate of the block column
 */
extern void update_state_block_col(Board *b, int xStart, int xStop);


/**
 * Compare block columns of two board for debugging purposes
 * @param b1        An initialized board
 * @param xStart1   Starting X-coordinate of the block column of the first board
 * @param xStop1    Stopping X-coordinate of the block column of the first board
 * @param b2        An initialized board
 * @param xStart2   Starting X-coordinate of the block column of the second board
 * @param xStop2    Stopping X-coordinate of the block column of the second board
 * @return 1 if the boards are identical, 0 else
 */
extern int compare_boards_block_col(Board *b1, int xStart1, int xStop1, Board *b2, int xStart2, int xStop2);


/**
 * Output a representation of a board for debugging purposes (excluding the ghost cells)
 * @param stream Stream on which to output the board
 * @param b An initialized board
 */
extern void output_board(FILE *stream, Board *b);


/**
 * Release the resources associated with a board.
 * @param b An initialized board
 */
extern void free_board(Board *b);

#endif //BOARD_H
