#include "dgemm_versions.h"
#include "cblas.h"

#include <assert.h>
#include <stdio.h>

/*
 * A scalar implementation of dgemm with 3 nested loops in the following order
 * row of C, col of C, row of B
 * that only supports the following operation:
 * C <- alpha * TransA * B + beta * C
 * with Order == CblasColMajor
 */
void cblas_dgemmScalarijk(const enum CBLAS_ORDER Order, const enum CBLAS_TRANSPOSE TransA,
                            const enum CBLAS_TRANSPOSE TransB, const int M, const int N,
                            const int K, const double alpha, const double *A,
                            const int lda, const double *B, const int ldb,
                            const double beta, double *C, const int ldc)
{
  assert(Order == CblasColMajor);
  assert(TransA == CblasTrans);
  assert(TransB == CblasNoTrans);

  double s;

  // TransA is MxK, A is KxM, B is KxN, C is MxN
  for (int row = 0; row < M; row++) {
    for (int col = 0; col < N; col++) {
      // s = sum(k=1toK) A_kr * B_kc
      s = A[lda * row] * B[ldb * col];
      for (int k = 1; k < K; k++) {
        s += A[lda * row + k] * B[ldb * col + k];
      }
      // C_rc = beta * C_rc + alpha * s
      C[ldc * col + row] = alpha * s + beta * C[ldc * col + row];
    }
  }
}

/*
 * Returns the number of floating operation of the implementation of
 * cblas_dgemmScalarijk for specified matrix sizes
 * M*N*(K + K-1 + 3) = M*N*(2K+2) flops
 * M*N mem write on array operands
 * M*N*(2K+1) mem read on array operands
 */
long cblas_dgemmScalarijk_flops(const int M, const int N, const int K) {
  return M*N*(2*K+2);
}

/*
 * A scalar implementation of dgemm with 3 nested loops in the following order
 * col of C, row of C, row of B
 * that only supports the following operation:
 * C <- alpha * TransA * B + beta * C
 * with Order == CblasColMajor
 */
void cblas_dgemmScalarjik(const enum CBLAS_ORDER Order, const enum CBLAS_TRANSPOSE TransA,
                            const enum CBLAS_TRANSPOSE TransB, const int M, const int N,
                            const int K, const double alpha, const double *A,
                            const int lda, const double *B, const int ldb,
                            const double beta, double *C, const int ldc)
{
  assert(Order == CblasColMajor);
  assert(TransA == CblasTrans);
  assert(TransB == CblasNoTrans);

  double s;

  // TransA is MxK, A is KxM, B is KxN, C is MxN
  for (int col = 0; col < N; col++) {
    for (int row = 0; row < M; row++) {
      // s = sum(k=1toK) A_kr * B_kc
      s = A[lda * row] * B[ldb * col];
      for (int k = 1; k < K; k++) {
        s += A[lda * row + k] * B[ldb * col + k];
      }
      // C_rc = beta * C_rc + alpha * s
      C[ldc * col + row] = alpha * s + beta * C[ldc * col + row];
    }
  }
}

/*
 * Returns the number of floating operation of the implementation of
 * cblas_dgemmScalarjik for specified matrix sizes
 * M*N*(K + K-1 + 3) = M*N*(2K+2) flops
 * M*N mem write on array operands
 * M*N*(2K+1) mem read on array operands
 */
long cblas_dgemmScalarjik_flops(const int M, const int N, const int K) {
  return M*N*(2*K+2);
}


/*
 * A scalar implementation of dgemm with 3 nested loops in the following order
 * row of B, row of C, col of C
 * that only supports the following operation:
 * C <- alpha * TransA * B + beta * C
 * with Order == CblasColMajor
 */
void cblas_dgemmScalarkij(const enum CBLAS_ORDER Order, const enum CBLAS_TRANSPOSE TransA,
                            const enum CBLAS_TRANSPOSE TransB, const int M, const int N,
                            const int K, const double alpha, const double *A,
                            const int lda, const double *B, const int ldb,
                            const double beta, double *C, const int ldc)
{
  assert(Order == CblasColMajor);
  assert(TransA == CblasTrans);
  assert(TransB == CblasNoTrans);

  // TransA is MxK, A is KxM, B is KxN, C is MxN
  for (int col = 0; col < N; col++) {
    for (int row = 0; row < M; row++) {
      C[ldc * col + row] = beta * C[ldc * col + row];
    }
  }
  for (int k = 0; k < K; k++) {
    for (int col = 0; col < N; col++) {
      for (int row = 0; row < M; row++) {
        C[ldc * col + row] += alpha * A[lda * row + k] * B[ldb * col + k];
      }
    }
  }
}

/*
 * Returns the number of floating operation of the implementation of
 * cblas_dgemmScalarkij for specified matrix sizes
 * M*N*(3K + 2) flops
 * M*N*(K+1) mem write on array operands
 * M*N*(3K+1) mem read on array operands
 */
long cblas_dgemmScalarkij_flops(const int M, const int N, const int K) {
  return M*N*(3*K+2);
}
