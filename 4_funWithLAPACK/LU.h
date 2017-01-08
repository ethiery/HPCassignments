#ifndef LU_H
#define LU_H

#include "distributedData.h"

/**
 * LU factorization without pivoting
 * A is a MxN matrix of leading dimension A
 */
void dgetrf2_nopiv(int M, int N, double *A, int lda);

/**
 * LU factorization without pivoting
 * Blocked version with block of size B using BLAS 3
 * A is a MxN matrix of leading dimension A
 */
void dgetrf_nopiv(int M, int N, double *A, int lda, int B);

/**
 * LU factorization without pivoting
 * Distributed version with :
 * - Column blocks of width dA->blockSize scattered among MPI nodes
 * - fine grain blocks of size fineBlockSize, which must be a divider of dA->blockSize
 * 
 * dA must be a NxN matrix scattered among MPI nodes with scatterMatrix()
 */
void pdgetrf_nopiv(DistributedMatrix *dA, int fineBlockSize);

/**
 * Solves a system of linear equations A*X = B using LU factorization 
 * computed by dgetrf_nopiv or dgetrf2_nopiv, and only BLAS 2 operations
 *
 * where LU is the factorizatioon of a general NxN matrix A of leading dimension lda,
 * and B is a vector of size N, overwritten with the solution
 */
void dgetrs2_nopiv(int N, double *LU, int lda, double *B);



#endif