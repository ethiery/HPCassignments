#include <assert.h>
#include <mkl.h>
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>


#include "util.h"

int main(int argc, char**argv)
{
  if (argc < 3)
  {
    fprintf(stderr, "Usage:\n./localGemm matrixApath matrixBpath ouputMatrixPath\n"
      "or\n./localGemm matrixApath matrixBpath");
    return EXIT_FAILURE;
  }

  double *A, *B;
  int M1, N1, M2, N2;
  struct timeval start, stop;
  parseMatrixFile(argv[1], &A, &M1, &N1);
  parseMatrixFile(argv[2], &B, &M2, &N2);

  if (N1 != M2)
  {
    fprintf(stderr, "Invalid matrix sizes\n");
    return EXIT_FAILURE;
  }

  if (A != NULL && B != NULL)
  {
    double *C = (double *) malloc(M1 * N2 * sizeof(double));

    if (C == NULL)
    {
      fprintf(stderr, "Error initializing result matrix\n");
      return EXIT_FAILURE;
    }
    gettimeofday(&start, NULL);
    cblas_dgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans,
                M1, N2, N1, 1, A, N1, B, N2, 0, C, N2);
    gettimeofday(&stop, NULL);
    printf("%lf\n", stop.tv_sec - start.tv_sec + 0.000001 * (stop.tv_usec - start.tv_usec));
    if (argc >= 4)
      writeMatrixFile(M1, N2, C, argv[3]);
    free(C);
  }

  if (A != NULL) free(A);
  if (B != NULL) free(B);
  return EXIT_SUCCESS;
}
