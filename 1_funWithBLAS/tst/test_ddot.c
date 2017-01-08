#include "benchtools.h"
#include "cblas.h"
#include "util.h"

#include <assert.h>
#include <float.h>
#include <math.h>

#define DDOT_NB_SAMPLES 10
#define DDOT_MIN_VEC_SIZE 50
#define DDOT_MAX_VEC_SIZE 1500000
#define DDOT_EXP_FACTOR 1.25

/*
 * Checks the result of a simple ddot operation
 */
int test_ddot(const int verbose)
{
  int n = 5;
  double *A = dvec(n);
  double *B = dvec(n);

  for (int i = 0; i < n; i++) {
    A[i] = i;
    B[i] = i%2 ? i+1 : -i-1;
  }

  if ((int) cblas_ddot(n, A, 1, B, 1) != -12) {
    printf("%f\n", cblas_ddot(n, A, 1, B, 1));
    FAIL("test_ddot: ddot returned an unexpected result\n");
  }
  dfree(A);
  dfree(B);

  n = 100000;
  A = dvec(n);
  B = dvec(n);
  for (int i = 0; i < n; i++) {
    A[i] = 1;
    B[i] = 1;
  }

  if ((int) cblas_ddot(n, A, 1, B, 1) != n) {
    printf("%f\n", cblas_ddot(n, A, 1, B, 1));
    FAIL("test_ddot: ddot returned an unexpected result\n");
  }
  dfree(A);
  dfree(B);

  SUCCESS("test_ddot: passed\n");
}

/*
 * Measures the performances of the ddot operation on vectors of different
 * sizes betwen MIN_VEC_SIZE and MAX_VEC_SIZE. Outputs the results on stdout
 * If cacheFlush is true, makes sure that the operands are not in the cache
 * before each operation
 */
void bench_ddot(const int cacheFlush)
{
  long totalCacheSize = getCacheSize(DATA_CACHE | UNIFIED_CACHE, ANY_LEVEL, 0);

  printf("#VectorSizeSFlopPerSInstructionsL1CacheMissesLLCacheMissesL1CacheMisses/InstructionLLCacheMisses/Instruction\n");

  for (int n = DDOT_MIN_VEC_SIZE; n < DDOT_MAX_VEC_SIZE; n = (int)n*DDOT_EXP_FACTOR) {
    // Measures are done NB_DDOT_SAMPLES times and only the fastest one is kept (see readme for explanations)
    double maxFlopPerS = 0, flopPerS = 0, duration = 0;
    long long instructionCount = -1, l1CacheMissCount = -1, llCacheMissCount = -1;

    // For each samples, ddot is performed a certain number of times,
    // to make sure that the duration timed is well above the clock precision.
    int nbIters = ceil(20.0*totalCacheSize / (16*n));
    // The results are stored in dot with alterning sign to avoid an overflows
    // Then dot is used, so that the compiler does not optimize out the loop
    double dot = 0;
    // Prepares a timer for nbIters iterations of ddot (2n-1 flops) on vectors of size n
    benchTimer *t = timer_init(2*n-1, nbIters);

    // Prepares operands. If we need to flush the caches before each ddot,
    // We actually need to allocate a set of vectors, that we will
    // iterate on starting from the end to cancel any prefetch effect
    // By initializing them in the same order, as the total size of the set
    // is a lot bigger than the size of the cache, we ensure that for each ddot
    // call, the operands won't be in the cache
    int nbSets = cacheFlush ? nbIters : 1;
    double **vecA = (double **) malloc(nbSets*sizeof(double *));
    double **vecB = (double **) malloc(nbSets*sizeof(double *));
    for (int setNo = nbSets-1; setNo >= 0; setNo--) {
      vecA[setNo] = dvec(n);
      vecB[setNo] = dvec(n);
      drandomize(n, 1, vecA[setNo], 1);
      drandomize(n, 1, vecB[setNo], 1);
    }

    for (int sampleNo = 0; sampleNo < DDOT_NB_SAMPLES; sampleNo++) {

      if (cacheFlush) {
        timer_start(t);
        for (int setNo = nbIters-1; setNo >= 0; setNo--) {
          dot += (setNo%2 ? 1 : -1) * cblas_ddot(n, vecA[setNo], 1, vecB[setNo], 1);
        }
        timer_stop(t);
      } else {
        // cache operands
        dot += cblas_ddot(n, vecA[0], 1, vecB[0], 1);
        timer_start(t);
        for (int i = 0; i < nbIters; i++) {
          dot += (i%2 ? 1 : -1) * cblas_ddot(n, vecA[0], 1, vecB[0], 1);
        }
        timer_stop(t);
      }

      assert(dot <= DBL_MAX);


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

    for (int setNo = nbSets-1; setNo >= 0; setNo--) {
      dfree(vecA[setNo]);
      dfree(vecB[setNo]);
    }
    free(vecA);
    free(vecB);
  }
}
