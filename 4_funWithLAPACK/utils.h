#ifndef UTILS_H
#define UTILS_H

#include "genericWrapper.h"

/** Command line parsing */

/**
 * Parses command line arguments and initialize parameters
 * Returns 0 in case of success, -1 else;
 */
int parse(int argc, char const *argv[], int *M, int *N, double *min, double *max, 
		   int *seed, int *coarseBlockSize, int *fineBlockSize, generic_dgetrf *func, char **name);

/** Test data initialization */

/** Allocates and initializes a MxN copy of leading dimension M of mat
 *  mat must be a MxN matrix of leading dimension ld
 */
double *copyMatrix(int M, int N, double *mat, int ld);

/**
 * Allocates and initializes a MxN matrix with doubles between min and max
 */
double *randomMatrix(int M, int N, double min, double max);


/**
 * Allocates and initializes a system of N linear equations and its solution:
 *
 *     1 1 0 0 ... 0                   1                2
 *     1 0 2 0 ... 0                   1                3
 *     1 0 0 3 ... 0                   1                4
 * A = .       .   .       times  X =  .     equals B = .
 *     1 0 0 0 ... n-1                 1                N
 *     1 0 0 0 ... 0                   1                1
 *
 * if Acopy, Bcopy and Xcopy are not null, then they are initialized
 * with copies of A, B, and X 
 *
 */
void initDebugSystem(int N, double **A, double **Acopy, double **B, double **Bcopy, double **X, double **Xcopy);

/**
 * Allocates and initializes a system of N random linear equations (with coefficients
 * between min and max):
 *
 * if Acopy and Bcopy are not null, then they are initialized
 * with copies of A, and B 
 *
 */
void initRandomTestSystem(int N, double min, double max, double **A, double **Acopy, double **B, double **Bcopy);

/** Accuracy measuring methods */

/**
 * Returns the maximal element of |A - LU|
 */
double maxElementwiseError(int M, int N, double *A, int lda, double *LU, int ldlu);

/**
 * Computes the normwise backward error on the solution X of a linear system A*X = B
 *
 * A is a general NxN matrix of leading dimension lda
 * X and B are vectors of size N
 */ 
double normwiseBackwardError(int N, double *A, int lda, double *X, double *B);

/**
 * Computes the partial normwise backward error on the solution X of a linear system A*X = B
 * Defined as:
 * 
 * min { eps : Ax = b + db, ||dB|| <= eps ||b|| }
 *
 *        = ||Ax - b|| / ||b||
 * 
 * where norms are 2-norms
 *
 * A is a general NxN matrix of leading dimension lda
 * X and B are vectors of size N
 */ 
double partialNormwiseBackwardError(int N, double *A, int lda, double *X, double *B);


/** Debug helper methods */

void printMatrix(int M, int N, double *mat, int ld);

#endif