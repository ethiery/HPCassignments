#include "cblas.h"
#include "dgemm_versions.h"

#include <assert.h>
#include <omp.h>
#include <stdio.h>

// #ifndef MYLIB_NUM_THREADS
// #define MYLIB_NUM_THREADS 10
// #endif

#define BLOCK_SIZE 2

#define min(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a < _b ? _a : _b; })

void cblas_dgemm(const enum CBLAS_ORDER Order, const enum CBLAS_TRANSPOSE TransA,
                 const enum CBLAS_TRANSPOSE TransB, const int M, const int N,
                 const int K, const double alpha, const double *A,
                 const int lda, const double *B, const int ldb,
                 const double beta, double *C, const int ldc)
{
  assert(Order == CblasColMajor);
  assert(TransA == CblasTrans);
  assert(TransB == CblasNoTrans);

  // multiply C by beta
  #pragma omp parallel for collapse(2)
  for (int col = 0; col < N; col++) {
    for (int row = 0; row < M; row++) {
      C[ldc * col + row] = beta * C[ldc * col + row];
    }
  }

  int nbRowBlocks = M/BLOCK_SIZE + (M%BLOCK_SIZE != 0 ? 1 : 0);
  int nbColBlocks = N/BLOCK_SIZE + (N%BLOCK_SIZE != 0 ? 1 : 0);
  int nbKBlocks = K/BLOCK_SIZE + (K%BLOCK_SIZE != 0 ? 1 : 0);

  #pragma omp parallel for collapse(2)
  for (int row = 0; row < nbRowBlocks; row++) {
    for (int col = 0; col < nbColBlocks; col++) {

      int rowStart = row * BLOCK_SIZE;
      int rowEnd = min((row+1) * BLOCK_SIZE, M);
      int colStart = col * BLOCK_SIZE;
      int colEnd = min((col+1) * BLOCK_SIZE, N);

      for (int k = 0; k < nbKBlocks; k++) {
        int kStart = k * BLOCK_SIZE;
        int kEnd = min((k+1) * BLOCK_SIZE, K);
        cblas_dgemmScalarjik(Order, TransA, TransB,
          rowEnd-rowStart, colEnd-colStart, kEnd-kStart, alpha,
          &A[lda*rowStart + kStart], lda,
          &B[ldb*colStart + kStart], ldb,
          1, &C[ldc*colStart + rowStart], ldc);
      }
    }
  }
}
