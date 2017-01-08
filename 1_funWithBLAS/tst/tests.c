#include <getopt.h>
#include <stdlib.h>
#include <stdio.h>

#include "benchtools.h"
#include "test_util.c"
#include "test_ddot.c"
#include "test_dgemm.c"

typedef int (*t_testFunc)(int);

t_testFunc testFuncs[] = {
  test_ddisp,
  test_ddot,
  test_dgemmScalar_alphaTransABplusBetaC_ijk,
  test_dgemmScalar_alphaTransABplusBetaC_jik,
  test_dgemmScalar_alphaTransABplusBetaC_kij,
  test_dgemmBloc_alphaTransABplusBetaC
};

void performAllTests() {
  int passed = 0;
  int nbTests = sizeof(testFuncs) / sizeof(t_testFunc);
  for (int i = 0; i < nbTests; i++) {
    if (testFuncs[i](1))
      passed++;
  }
  printf("%d/%d tests passed\n", passed, nbTests);
}

void printUsage() {
  printf("Usage: ./myLib -t   ./myLib -i   ./myLib -b functionName [-f]\n"
         "-t : performs all available tests\n"
         "-i : prints informations on processors cache\n"
         "-b funcName : benchmark the specified function (ddot, "
         "dgemm_scalar_ijk, dgemm_scalar_jik, dgemm_scalar_kij, dgemm_vectorial dgemm_bloc)\n"
         "-f flush the proc caches before each call to the benchmarked function\n"
  );
}

int main(int argc, char**argv)
{
  int opt;

  char mode = 'u';
  char func[20];
  int flush = 0;

  while ((opt = getopt(argc, argv, "tib:f")) != -1) {
    switch (opt) {
      case 'b': // benchmark, value is the name of the function to bench
        mode = 'b';
        strncpy(func, optarg, 20);
        break;
      case 't':
        mode = 't';
        break;
      case 'f':
        flush = 1;
        break;
      case 'i':
        mode = 'i';
        break;
      default:
        abort();
    }
  }

  switch(mode) {
    case 'b':
      if (strcmp(func, "ddot") == 0) {
        bench_ddot(flush);
      } else if (strcmp(func, "dgemm_scalar_ijk") == 0) {
        bench_dgemmScalarijk(flush);
      } else if (strcmp(func, "dgemm_scalar_jik") == 0) {
        bench_dgemmScalarjik(flush);
      } else if (strcmp(func, "dgemm_scalar_kij" ) == 0) {
        bench_dgemmScalarkij(flush);
      } else if (strcmp(func, "dgemm_bloc") == 0) {
        bench_dgemmBloc(flush);
      } else {
        printUsage();
      }
      break;
    case 't':
      performAllTests();
      break;
    case 'i':
      getCacheSize(DATA_CACHE, ANY_LEVEL, 1);
      break;
    case 'u':
      printUsage();
      break;
  }
}
