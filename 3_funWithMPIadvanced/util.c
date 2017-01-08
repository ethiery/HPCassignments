#include "util.h"

#include <assert.h>
#include <malloc.h>
#include <stdio.h>

void parseMatrixFile(char *inputFilePath, double **mat, int *M, int *N)
{
  FILE *f = fopen(inputFilePath, "r");
  if (f == NULL)
  {
    fprintf(stderr, "Invalid matrix file path(s)\n");
    return;
  }

  if (fscanf(f, "%d %d\n", M, N) != 2 || *M < 1 || *N < 1)
  {
    fprintf(stderr, "Invalid matrix file\n");
    fclose(f);
    return;
  }

  *mat = (double *) malloc(*M * *N * sizeof(double));
  if (*mat == NULL)
  {
    fprintf(stderr, "Error initializing matrix\n");
    fclose(f);
    return;
  }

  for (int i = 0; i < *M * *N; i++)
  {
    fscanf(f, "%*[ \n\t]");
    if (fscanf(f, "%lf", (*mat) + i) != 1)
    {
      fprintf(stderr, "Invalid matrix file\n");
      fclose(f);
      return;
    }
  }
}

void writeMatrixFile(int M, int N, double *mat, char *outputFilePath)
{
  FILE *f = fopen(outputFilePath, "w");
  assert(f != NULL);

  assert(fprintf(f, "%d %d\n", M, N) > 0);

  for (int i = 0; i < M; i++)
  {
    for (int j = 0; j < N; j++)
    {
      assert(fprintf(f, "%lf ", mat[i * N + j]) > 0);
    }
    assert(fprintf(f, "\n") > 0);
  }
}
