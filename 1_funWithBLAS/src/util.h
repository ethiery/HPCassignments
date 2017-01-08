#ifndef UTIL_H
#define UTIL_H

#include <stdio.h>
#include <time.h>
#include <stdlib.h>

/*
 * Allocates a new vector of size n. (32 bytes aligned)
 */
double *dvec(const int n);

/*
 * Allocates a new matrix of size m*n (column major, each column is 32 bytes aligned)
 * (m rows, n cols).
 */
double *dmat(const int m, const int n);

/*
 * Frees a previously allocated vector or matrix a
 */
void dfree(double *A);

/*
 * Displays the sub matrix of size m*n (m rows, n cols) pointed by a
 * of a matrix of leading dimension lda on the specified stream
 */
void ddisp(const int m, const int n, const double *A,
           const int lda, FILE *stream);

/*
 * Fills the sub matrix of size m*n pointed by a
 * of a matrix of leading dimension lda with random
 * values
 */
void drandomize(const int m, const int n, double *A, const int lda);


#endif //UTIL_H
