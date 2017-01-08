#include <cblas.h>
#include <stdio.h>
#include <assert.h>
/*
  
 * y = alpha*A*x + beta*y,
 * where alpha and beta are scalars, x and y are vectors and A is an
 * m by n matrix.
 */

void cblas_dgemv(const enum CBLAS_ORDER order,
                 const enum CBLAS_TRANSPOSE TransA, const int M, const int N,
                 const double alpha, const double *A, const int lda,
                 const double *X, const int incX, const double beta,
		 double *Y, const int incY)
  
{
  assert(order == CblasColMajor);
  assert(TransA == CblasNoTrans);
  int i;
  for (i = 0; i < N; ++i)
    {
      Y[i*incY] += beta* Y[i*incY] + alpha* cblas_ddot(N, A[i] , lda , X , incX);
    }
  
}
