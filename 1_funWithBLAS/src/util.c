#include "util.h"
#include <stdlib.h>

double *dvec(const int n)
{
  return (double *) aligned_alloc(4*sizeof(double), n * sizeof(double));
  // return (double *) malloc(n * sizeof(double));
}

void drandomize(const int m, const int n, double * A, const int lda)
{
  srand(time(NULL));
  for (int col = 0; col < n; col++) {
    for (int row = 0; row < m; row++) {
      A[lda * col + row] = rand();
    }
  }
}

double *dmat(const int m, const int n)
{
  return (double *) aligned_alloc(4*sizeof(double), n * m * sizeof(double));
  // return (double *) malloc(n * m * sizeof(double));
}

void dfree(double *const A)
{
  free(A);
}

void ddisp(const int m, const int n, const double *A,
           const int lda, FILE *stream)
{
  for (int row = 0; row < m; row++) {
    for (int col = 0; col < n; col++) {
      fprintf(stream, "%.3e ", A[lda * col + row]);
    }
    fprintf(stream, "\n");
  }
}
