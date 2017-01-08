#include "cblas.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

// The default is to use scalar code
#ifndef VECTORIZED
#define VECTORIZED 0
#endif

// The default is to use sequential code
#ifndef PARALLEL_THRESH
#define PARALLEL_THRESH INT_MAX
#endif

typedef double vec4d __attribute__ ((vector_size (4*sizeof(double))));

/*
 * Performs the dot product between N elements of vectors X and Y,
 * using an increment between elements of incX (resp. incY)
 * N must be > 0
 */
double cblas_ddot(const int N, const double *X, const int incX,
                  const double *Y, const int incY)
{

  // vectorized version using gcc vector extension support
  if (incX == 1 && incY == 1 && VECTORIZED)
  {
    vec4d vSum = { 0 } ;
    vec4d *vecX = (vec4d *)X;
    vec4d *vecY = (vec4d *)Y;
    int K = N/4;
    int R = N%4;
    int i;

    // parallel version using open mp
    if (N > PARALLEL_THRESH)
    {
      #pragma omp parallel for \
        default(shared) private(i) \
        reduction(+:vSum)
      for (i = 0; i < K; i++) {
        vSum = vSum + vecX[i] * vecY[i];
      }
    }
    // sequential version
    else {
      for (i = 0; i < K; i++) {
        vSum = vSum + vecX[i] * vecY[i];
      }
    }
    vSum = vSum + (vec4d) {
      R >= 1 ? X[4*K] * Y[4*K] : 0.0,
      R >= 2 ? X[4*K+1] * Y[4*K+1] : 0.0,
      R >= 3 ? X[4*K+2] * Y[4*K+2] : 0.0,
      0.0
    };
    return vSum[0] + vSum[1] + vSum[2] + vSum[3];
  }
  // scalar version
  else
  {
    double res = X[0] * Y[0];
    int i;
    // parallel version using open mp
    if (N > PARALLEL_THRESH)
    {
      #pragma omp parallel for \
        default(shared) private(i) \
        reduction(+:res)
      for (i = 1; i < N; i++) {
        res += X[i*incX] * Y[i*incY];
      }
    }
    // sequential version
    for (i = 1; i < N; i++) {
      res += X[i*incX] * Y[i*incY];
    }
    return res;
  }
}

// Old version using intrinsics
// int K = N/4;
// int R = N%4;
// __m256d vSum = _mm256_set1_pd(0.0);
// int i;
// for (i = 0; i < K; i++) {
//   // vSum = _mm256_add_pd(vSum, _mm256_mul_pd(_mm256_load_pd(X+4*i), _mm256_load_pd(Y+4*i)));
//   vSum = _mm256_fmadd_pd(_mm256_load_pd(X+4*i), _mm256_load_pd(Y+4*i), vSum);
// }
// // Adds left overs
// vSum = _mm256_add_pd(vSum, _mm256_set_pd(R >= 1 ? X[4*K]*Y[4*K] : 0.0,
//   R >= 2 ? X[4*K+1]*Y[4*K+1] : 0.0,
//   R >= 3 ? X[4*K+2]*Y[4*K+2] : 0.0,
//   0.0));
//   double sums[4]  __attribute__ ((aligned (4*sizeof(double))));
//   _mm256_store_pd(sums, vSum);
//   return sums[0] + sums[1] + sums[2] + sums[3];
