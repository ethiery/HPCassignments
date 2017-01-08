#ifndef DGEMM_VERSIONS_H
#define DGEMM_VERSION_H

#include "cblas.h"

void cblas_dgemmScalarijk(const enum CBLAS_ORDER Order, const enum CBLAS_TRANSPOSE TransA,
                          const enum CBLAS_TRANSPOSE TransB, const int M, const int N,
                          const int K, const double alpha, const double *A,
                          const int lda, const double *B, const int ldb,
                          const double beta, double *C, const int ldc);

void cblas_dgemmScalarjik(const enum CBLAS_ORDER Order, const enum CBLAS_TRANSPOSE TransA,
                          const enum CBLAS_TRANSPOSE TransB, const int M, const int N,
                          const int K, const double alpha, const double *A,
                          const int lda, const double *B, const int ldb,
                          const double beta, double *C, const int ldc);

void cblas_dgemmScalarkij(const enum CBLAS_ORDER Order, const enum CBLAS_TRANSPOSE TransA,
                          const enum CBLAS_TRANSPOSE TransB, const int M, const int N,
                          const int K, const double alpha, const double *A,
                          const int lda, const double *B, const int ldb,
                          const double beta, double *C, const int ldc);

long cblas_dgemmScalarijk_flops(const int M, const int N, const int K);

long cblas_dgemmScalarjik_flops(const int M, const int N, const int K);

long cblas_dgemmScalarkij_flops(const int M, const int N, const int K);

#endif
