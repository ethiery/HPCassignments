#include <stdio.h>
#include<assert.h>
#include "cblas.h"
/*
  
 * A = alpha*x*y**T + A;  
 * where alpha is a scalar, x is an m element vector, y is an n element
 *  vector and A is an m by n matrix.
 
 */
void cblas_dger(const enum CBLAS_ORDER order, const int M, const int N,
                const double alpha, const double *X, const int incX,
		const double *Y, const int incY, double *A, const int lda)

{
  assert(order == CblasColMajor);
  int i,j;
  for (i = 0; i < M; ++i)
    {
      for (j = 0; j < N; ++j)
	{
	  A[j*lda + i] += alpha * X[i*incX] * Y[j*incY];
	}
      
    }
}
