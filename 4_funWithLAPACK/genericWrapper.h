#ifndef GENERIC_WRAPPER_H
#define GENERIC_WRAPPER_H

#include <mkl.h>

typedef double (*generic_dgetrf) (int M, int N, double *LU, double ld, lapack_int *ipiv, int coarseBlockSize, int fineBlockSize, int root);

double dgetrf_MKL(int M, int N, double *LU, double ld, lapack_int *ipiv, int coarseBlockSize, int fineBlockSize, int root);
double dgetrf_customBlas2Nopiv(int M, int N, double *LU, double ld, lapack_int *ipiv, int coarseBlockSize, int fineBlockSize, int root);
double dgetrf_customBlas3Nopiv(int M, int N, double *LU, double ld, lapack_int *ipiv, int coarseBlockSize, int fineBlockSize, int root);
double dgetrf_customDistributedNopiv(int M, int N, double *LU, double ld, lapack_int *ipiv, int coarseBlockSize, int fineBlockSize, int root);

#endif