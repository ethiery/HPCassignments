#include "benchtools.h"
#include "cblas.h"
#include "dgemm_versions.h"
#include "util.h"


#include <assert.h>
#include <math.h>
#include <string.h>


#define DGEMM_NB_SAMPLES 10
#define DGEMM_MIN_MAT_SIZE 100
#define DGEMM_MAX_MAT_SIZE 1000
#define DGEMM_EXP_FACTOR 1.26

#define flCacheLn(mem)__asm__ __volatile__\
("clflush %0" :: "m" (*((char *)(mem))))

// An implementation of dgemm
typedef void (*t_dgemmFunc)(const enum CBLAS_ORDER, const enum CBLAS_TRANSPOSE,
                   const enum CBLAS_TRANSPOSE, const int, const int, const int,
                   const double, const double *, const int, const double *,
                   const int, const double, double *, const int);

typedef long (*t_dgemmFunc_flops)(const int, const int, const int);

// Some tests data

int M = 4, N = 3, K = 5;

double At[] = { 0, -1, -2, -3, -4,
                1,  0, -1, -2, -3,
                2,  1,  0, -1, -2,
                3,  2,  1,  0, -1 }; // 5x4

double B[] = { 0, 1, 2, 3, 4,
               1, 2, 3, 4, 5,
               2, 3, 4, 5, 6 }; // 5x3

double C[] = { 0, 1, 0, 1,
               1, 0, 1, 0,
               0, 1, 0, 1 }; // 4x3


// The most generic version of a dgemm test
int test_dgemm(const int verbose, t_dgemmFunc dgemmFunc, const char *funcName,
               const enum CBLAS_ORDER Order, const enum CBLAS_TRANSPOSE TransA,
               const enum CBLAS_TRANSPOSE TransB, const int M, const int N,
               const int K, const double alpha, const double *A,
               const int lda, const double *B, const int ldb,
               const double beta, double *C, const int ldc, double *expected)
{
  dgemmFunc(Order, TransA, TransB, M, N, K, alpha, A, lda, B, ldb, beta, C, ldc);
  double epsilon = 1e-10;

  for (int i = 0; i < M*N; i++) {
    if (abs(C[i] - expected[i]) > epsilon) {
      ddisp(M, N, C, M, stdout);
      ddisp(M, N, expected, M, stdout);
      FAIL("%s: dgemm returned an unexpected result\n", funcName);
    }
  }

  SUCCESS("%s: passed\n", funcName);
}

// Another (a bit less) generic version of the test, for the
// case C <- alpha TranS B + beta C
int test_dgemm_alphaTransABplusBetaC(const int verbose,
                                           t_dgemmFunc dgemmFunc,
                                           const char *funcName)
{
  double *copyC = dmat(M, N);
  memcpy(copyC, C, M*N*sizeof(double));
  double expected[12] = { -15,   -8.5,    -5, 1.5,
                          -18.5, -12.5, -3.5, 2.5,
                          -25,   -13.5,   -5, 6.5 };
  int r = test_dgemm(verbose, dgemmFunc, funcName,
                     CblasColMajor, CblasTrans, CblasNoTrans, M, N, K, 0.5,
                     At, K, B, K, 1.5, copyC, M, expected);
  dfree(copyC);
  return r;
}

// Tests specific to different versions of dgemm, and different cases described
// by the name of the tests

int test_dgemmScalar_alphaTransABplusBetaC_ijk(const int verbose)
{
  return test_dgemm_alphaTransABplusBetaC(verbose, cblas_dgemmScalarijk,
    "test_dgemmScalar_alphaTransABplusBetaC_ijk");
}

int test_dgemmScalar_alphaTransABplusBetaC_jik(const int verbose)
{
  return test_dgemm_alphaTransABplusBetaC(verbose, cblas_dgemmScalarjik,
    "test_dgemmScalar_alphaTransABplusBetaC_jik");
}

int test_dgemmScalar_alphaTransABplusBetaC_kij(const int verbose)
{
  return test_dgemm_alphaTransABplusBetaC(verbose, cblas_dgemmScalarkij,
    "test_dgemmScalar_alphaTransABplusBetaC_kij");
}

int test_dgemmBloc_alphaTransABplusBetaC(const int verbose)
{
  return test_dgemm_alphaTransABplusBetaC(verbose, cblas_dgemm,
    "test_dgemmBloc_alphaTransABplusBetaC");
}

/*
 * Measures the performances of the specified implementation of dgemm
 * on C <- alpha * TransA B + beta C
 * on square matrices of sizes between
 * MIN_MAT_SIZE and MAX_MAT_SIZE. Outputs the results on stdout
 * If cacheFlush is true, makes sure that the operands are not in the cache
 * before each operation
 */
void bench_dgemm(const int cacheFlush, t_dgemmFunc dgemmFunc,
                 const char *funcName, t_dgemmFunc_flops dgemmFuncFlops)
{
  printf("#MatSize S FlopPerS Instructions L1CacheMisses LLCacheMisses L1CacheMisses/Instruction LLCacheMisses/Instruction\n");

  for (int n = DGEMM_MIN_MAT_SIZE; n < DGEMM_MAX_MAT_SIZE; n = (int)n*DGEMM_EXP_FACTOR) {
    // Measures are done NB_DGEMM_SAMPLES times and only the fastest one is kept (see readme for explanations)
    double maxFlopPerS = 0, flopPerS = 0, duration = 0;
    long long instructionCount = -1, l1CacheMissCount = -1, llCacheMissCount = -1;

    benchTimer *t = timer_init(dgemmFuncFlops(n, n, n), 1);
    double *matA = dmat(n, n);
    double *matB = dmat(n, n);
    double *matC = dmat(n, n);

    for (int sampleNo = 0; sampleNo < DGEMM_NB_SAMPLES; sampleNo++) {
      drandomize(n, n, matA, n);
      drandomize(n, n, matB, n);
      drandomize(n, n, matC, n);

      if (cacheFlush) {
        // Make sure the operands are not in the cache
        for (int i = 0; i < n*n; i++) {
          flCacheLn(matA+i);
          flCacheLn(matB+i);
          flCacheLn(matB+i);
        }
      }

      timer_start(t);
      dgemmFunc(CblasColMajor, CblasTrans, CblasNoTrans, n, n, n, 3.14,
                matA, n, matB, n, 1.44, matC, n);
      timer_stop(t);

      flopPerS = timer_getFlopPerS(t);
      duration = flopPerS > maxFlopPerS ? timer_getDuration(t) : duration;
      instructionCount = flopPerS > maxFlopPerS ? timer_getInstructionCount(t) : instructionCount;
      l1CacheMissCount = flopPerS > maxFlopPerS ? timer_getL1CacheMissCount(t) : l1CacheMissCount;
      llCacheMissCount = flopPerS > maxFlopPerS ? timer_getLlCacheMissCount(t) : llCacheMissCount;
      maxFlopPerS = flopPerS > maxFlopPerS ? flopPerS : maxFlopPerS;

    }
    printf("%d %.12e %.10e %lld %lld %lld %.5e %.5e\n",
      n, duration, maxFlopPerS, instructionCount, l1CacheMissCount, llCacheMissCount,
      (double)l1CacheMissCount/instructionCount, (double)llCacheMissCount/instructionCount);

    dfree(matA);
    dfree(matB);
    dfree(matC);

  }
}

// Bench specific versions of dgemm
void bench_dgemmScalarijk(const int cacheFlush)
{
  bench_dgemm(cacheFlush, cblas_dgemmScalarijk, "cblas_dgemmScalarijk", cblas_dgemmScalarijk_flops);
}

void bench_dgemmScalarjik(const int cacheFlush)
{
  bench_dgemm(cacheFlush, cblas_dgemmScalarjik, "cblas_dgemmScalarjik", cblas_dgemmScalarjik_flops);
}

void bench_dgemmScalarkij(const int cacheFlush)
{
  bench_dgemm(cacheFlush, cblas_dgemmScalarkij, "cblas_dgemmScalarkij", cblas_dgemmScalarkij_flops);
}

void bench_dgemmBloc(const int cacheFlush)
{
  bench_dgemm(cacheFlush, cblas_dgemm, "cblas_dgemmBloc", cblas_dgemmScalarjik_flops);
}
