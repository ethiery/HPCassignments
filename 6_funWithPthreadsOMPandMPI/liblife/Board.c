#include <malloc.h>
#include <math.h>
#include <mkl.h>
#include <string.h>
#include <assert.h>

#include "Board.h"

/* Readability macros */


/**
 * Update the state of a cell given its current state and its current number of neighbours
 * @param b (Initialized) board to update
 * @param x X-coordinate of the cell to update
 * @param y Y-coordinate of the cell to update
 */
#define update_state(b, x, y)\
    b->arr[(x) * b->ld + (y)] = STATE_CHANGES[(int)(b->arr[(x) * b->ld + (y)] * 9 + b->nbNgbs[(x) * b->Ny + (y)])]

/**
 * Copy the state of a source cell in a destination cell
 * @param b    (Initialized) board to update
 * @param xDst X-coordinate of the destination cell
 * @param yDst Y-coordinate of the destination cell
 * @param xSrc X-coordinate of the source cell
 * @param ySrc Y-coordinate of the source cell
 */
#define copy_state(b, xDst, yDst, xSrc, ySrc)\
    b->arr[(xDst) * b->ld + (yDst)] = b->arr[(xSrc) * b->ld + (ySrc)]


/* Public definitions */


void init_board(Board *b, int Nx, int Ny)
{
    b->Nx = Nx;
    b->Ny = Ny;
    b->ld = b->Ny + 2;
    b->arr = (float *) calloc((size_t) b->ld * b->ld, sizeof(float));
    assert(b->arr != NULL);
    b->arr = b->arr + b->ld + 1;
    b->nbNgbs = (float *) calloc((size_t) b->Ny * b->Ny, sizeof(float));
    assert(b->nbNgbs != NULL);
}


void set_col(Board *b, int x, int val)
{
    for (int y = -1; y <= b->Ny; y++)
        b->arr[x * b->ld + y] = val;
}


void set_row(Board *b, int y, int val)
{
    for (int x = -1; x <= b->Nx; x++)
        b->arr[x * b->ld + y] = val;
}


void count_neighbours_col(Board *b, int x)
{
    vsAdd(b->Ny, &(b->arr[x * b->ld - 1]), &(b->arr[x * b->ld + 1]), &(b->nbNgbs[x * b->Ny])); // top and bottom neighbours
    cblas_saxpy(b->Ny, 1, &(b->arr[(x-1) * b->ld - 1]), 1, &(b->nbNgbs[x * b->Ny]), 1); // top left neighbours
    cblas_saxpy(b->Ny, 1, &(b->arr[(x-1) * b->ld]), 1, &(b->nbNgbs[x * b->Ny]), 1); // left neighbours
    cblas_saxpy(b->Ny, 1, &(b->arr[(x-1) * b->ld + 1]), 1, &(b->nbNgbs[x * b->Ny]), 1); // bottom left neighbours
    cblas_saxpy(b->Ny, 1, &(b->arr[(x+1) * b->ld - 1]), 1, &(b->nbNgbs[x * b->Ny]), 1); // top left neighbours
    cblas_saxpy(b->Ny, 1, &(b->arr[(x+1) * b->ld]), 1, &(b->nbNgbs[x * b->Ny]), 1); // left neighbours
    cblas_saxpy(b->Ny, 1, &(b->arr[(x+1) * b->ld + 1]), 1, &(b->nbNgbs[x * b->Ny]), 1); // bottom left neighbours
}


void update_state_col(Board *b, int x)
{
    for (int y = 0; y < b->Ny; y++)
        update_state(b, x, y);

    copy_state(b, x,  -1 , x, b->Ny - 1);  // top ghost cell
    copy_state(b, x, b->Ny, x,     0 );    // bottom ghost cell
}


void copy_state_col(Board *b, int xDst, int xSrc)
{
    memcpy(&(b->arr[xDst * b->ld - 1]), &(b->arr[xSrc *b->ld - 1]), b->ld * sizeof(float));
}


void count_neighbours_block_col(Board *b, int xStart, int xStop, int blockSize)
{
    int y;
    for (y = 0; y + blockSize <= b->Ny; y += blockSize)
    {
        #pragma omp for schedule(static)
        for (int x = xStart; x < xStop; x++)
        {
            vsAdd(blockSize, &(b->arr[x * b->ld + (y-1)]), &(b->arr[x * b->ld + (y+1)]), &(b->nbNgbs[x * b->Ny + y])); // top and bottom neighbours
            cblas_saxpy(blockSize, 1, &(b->arr[(x-1) * b->ld + (y-1)]), 1, &(b->nbNgbs[x * b->Ny + y]), 1); // top left neighbours
            cblas_saxpy(blockSize, 1, &(b->arr[(x-1) * b->ld +   y  ]), 1, &(b->nbNgbs[x * b->Ny + y]), 1); // left neighbours
            cblas_saxpy(blockSize, 1, &(b->arr[(x-1) * b->ld + (y+1)]), 1, &(b->nbNgbs[x * b->Ny + y]), 1); // bottom left neighbours
            cblas_saxpy(blockSize, 1, &(b->arr[(x+1) * b->ld + (y-1)]), 1, &(b->nbNgbs[x * b->Ny + y]), 1); // top left neighbours
            cblas_saxpy(blockSize, 1, &(b->arr[(x+1) * b->ld +   y  ]), 1, &(b->nbNgbs[x * b->Ny + y]), 1); // left neighbours
            cblas_saxpy(blockSize, 1, &(b->arr[(x+1) * b->ld + (y+1)]), 1, &(b->nbNgbs[x * b->Ny + y]), 1); // bottom left neighbours
        }
    }

    int left = b->Ny - y;
    if (left > 0)
    {
        #pragma omp for schedule(static)
        for (int x = xStart; x < xStop; x++)
        {
            vsAdd(left, &(b->arr[x * b->ld + (y - 1)]), &(b->arr[x * b->ld + (y + 1)]),
                  &(b->nbNgbs[x * b->Ny + y])); // top and bottom neighbours
            cblas_saxpy(left, 1, &(b->arr[(x-1) * b->ld + (y-1)]), 1, &(b->nbNgbs[x * b->Ny + y]),
                        1); // top left neighbours
            cblas_saxpy(left, 1, &(b->arr[(x-1) * b->ld + y]), 1, &(b->nbNgbs[x * b->Ny + y]),
                        1); // left neighbours
            cblas_saxpy(left, 1, &(b->arr[(x-1) * b->ld + (y+1)]), 1, &(b->nbNgbs[x * b->Ny + y]),
                        1); // bottom left neighbours
            cblas_saxpy(left, 1, &(b->arr[(x+1) * b->ld + (y-1)]), 1, &(b->nbNgbs[x * b->Ny + y]),
                        1); // top left neighbours
            cblas_saxpy(left, 1, &(b->arr[(x+1) * b->ld + y]), 1, &(b->nbNgbs[x * b->Ny + y]),
                        1); // left neighbours
            cblas_saxpy(left, 1, &(b->arr[(x+1) * b->ld + (y+1)]), 1, &(b->nbNgbs[x * b->Ny + y]),
                        1); // bottom left neighbours
        }
    }
}


void update_state_block_col(Board *b, int xStart, int xStop)
{
#pragma omp for schedule(static)
    for (int x = xStart; x < xStop; x++)
        update_state_col(b, x);
}


int compare_boards_block_col(Board *b1, int xStart1, int xStop1, Board *b2, int xStart2, int xStop2)
{
    if ((b1->Ny != b2->Ny) || ((xStop1-xStart1) != (xStop2-xStart2)))
        return 0;

    for (int x = 0; x < (xStop1 - xStart1); x++)
        for (int y = 0; y < b1->Ny; y++)
            if (fabsf(b1->arr[(xStart1 + x) * b1->ld + y] - b2->arr[(xStart2 + x) * b2->ld + y]) > 0.5)
                return 0;

    return 1;
}


void output_board(FILE *stream, Board *b)
{
    for (int y = -1; y <= b->Ny; y++)
    {
        for (int x = -1; x <= b->Nx; x++)
            fprintf(stream, b->arr[x * b->ld + y] > 0.5 ? "X" : ".");

        fprintf(stream, "\n");
    }
}


void free_board(Board *b)
{
    free(b->nbNgbs);
    b->arr = b->arr - b->ld - 1;
    free(b->arr);
}