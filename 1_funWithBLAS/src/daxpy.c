#include "cblas.h"

/*
  
 *daxpy takes two vectors X and Y and do the following operation:
 * Y = alpha*X + Y 
 
 */
void cblas_daxpy(const int N, const double alpha, const double *X,
                 const int incX, double *Y, const int incY)
{
  int j;
  for (j = 0; j < N; ++j)
    {
      Y[j*incY] += alpha * X[j*incX]; 
    }
}
