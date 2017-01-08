#include <assert.h>
#include <getopt.h>
#include <math.h>
#include <mkl.h>
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#include "util.h"

typedef struct ScatterGatherInfo
{
  MPI_Datatype MPI_BLOCK;
  int blockSize;
  int gridSize;
  int *counts;
  int *displs;
} ScatterGatherInfo;

/**
 * Parses command line arguments.
 * Initialize matrices A and B with the content of the specified files.
 * Initializes blockSize depending on the number of MPI processes.
 *
 * After a call to parse, for process 0:
 * - in case of success, blockSize is > 0, A and B are allocated and initialized
 * - in case of failure, blockSize is 0, A and B are NULL
 */
void parse(int argc, char **argv, double **A, double **B, int *blockSize, int *gridSize)
{
  int rank, size, M1, N1, M2, N2;
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  *gridSize = sqrtl(size);

  if (rank == 0)
  {
    *A = NULL;
    *B = NULL;
    *blockSize = 0;

    if (argc < 3)
    {
    fprintf(stderr, "Usage:\n./localGemm matrixApath matrixBpath ouputMatrixPath\n"
      "or\n./localGemm matrixApath matrixBpath");
    }
    else if (*gridSize * *gridSize != size)
    {
      fprintf(stderr, "The number of MPI processes must be a square number\n");
    }
    else
    {
      parseMatrixFile(argv[1], A, &M1, &N1);
      if (*A != NULL)
      {
        if (M1 != N1)
        {
          fprintf(stderr, "Invalid first operand size: only square matrices are supported");
          free(*A);
        }
        else if (M1 % *gridSize != 0)
        {
          fprintf(stderr, "Invalid first operand size: must be a multiple of sqrt(nbProcesses)");
          free(*A);
        }
        else
        {
          parseMatrixFile(argv[2], B, &M2, &N2);
          if (*B == NULL)
          {
            free(*A);
            *A = NULL;
          }
          else if (M2 != M1 || N2 != N1)
          {
            fprintf(stderr, "Incompatible operand sizes: %dx%d and %dx%d", M1, M1, M2, M2);
            free(*A);
            free(*B);
            *A = *B = NULL;
          }
          else
          {            
            *blockSize = M1 / *gridSize;
          }
        }
      }
    }
  }

  MPI_Bcast(blockSize, 1, MPI_INT, 0, MPI_COMM_WORLD);
}

/**
 * Writes result NxN matrix A in the specified file,
 * then frees global matrices A and B
 */
void writeResult(int N, double *A, double *B, char *filePath)
{
  int rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  if (rank == 0)
  {
    writeMatrixFile(N, N, A, filePath);
    free(A);
    free(B);
  }
}

/*
 * Given a ScatterGatherInfo object containing the block size and
 * the grid size, initialize all the other info needed to scatter and gather
 */
ScatterGatherInfo *initScatterGatherInfo(int blockSize, int gridSize)
{
  ScatterGatherInfo *sgInfo = (ScatterGatherInfo *) malloc(sizeof(ScatterGatherInfo));
  sgInfo->blockSize = blockSize;
  sgInfo->gridSize = gridSize;
  int matSize = gridSize * blockSize;

  MPI_Datatype MPI_SUBARRAY;
  int matSizes[2] = { matSize, matSize };
  int blockSizes[2] = { blockSize, blockSize };
  int blockStarts[2] = { 0, 0 };
  MPI_Type_create_subarray(2, matSizes, blockSizes, blockStarts, MPI_ORDER_C, MPI_DOUBLE, &MPI_SUBARRAY);
  MPI_Type_create_resized(MPI_SUBARRAY, 0, sizeof(double), &(sgInfo->MPI_BLOCK));
  MPI_Type_commit(&(sgInfo->MPI_BLOCK));

  sgInfo->counts = (int *) malloc(gridSize * gridSize * sizeof(int));
  sgInfo->displs = (int *) malloc(gridSize * gridSize * sizeof(int));

  for (int i = 0; i < gridSize; i++)
    for (int j = 0; j < gridSize; j++)
      {
        sgInfo->displs[i * gridSize + j] = i * matSize * blockSize + j * blockSize;
        sgInfo->counts[i * gridSize + j] = 1;
      }

  return sgInfo;
}

void freeScatterGatherInfo(ScatterGatherInfo *sgInfo)
{
  free(sgInfo->counts);
  free(sgInfo->displs);
  free(sgInfo);
}

/**
 * Scatters matrices A and B of process 0 among all
 * the processes blockA and blockB.
 *
 * After a call to scatter:
 * - the matrices A and B of proccess 0 are freed
 * - the matrices blockA and blockB of all processe are allocated and initialized
 */
void scatter(ScatterGatherInfo *sgInfo, double *A, double *B, double *blockA, double *blockB)
{
  // Scatter blocks
  MPI_Scatterv(A, sgInfo->counts, sgInfo->displs, sgInfo->MPI_BLOCK,
               blockA, sgInfo->blockSize * sgInfo->blockSize, MPI_DOUBLE,
               0, MPI_COMM_WORLD);
  MPI_Scatterv(B, sgInfo->counts, sgInfo->displs, sgInfo->MPI_BLOCK,
               blockB, sgInfo->blockSize * sgInfo->blockSize, MPI_DOUBLE,
               0, MPI_COMM_WORLD);

  MPI_Barrier(MPI_COMM_WORLD);
}

/**
 * Gathers matrices blockC in C of process 0.
 */
void gather(ScatterGatherInfo *sgInfo, double *C, double *blockC)
{
  MPI_Gatherv(blockC, sgInfo->blockSize * sgInfo->blockSize, MPI_DOUBLE,
              C, sgInfo->counts, sgInfo->displs, sgInfo->MPI_BLOCK,
              0, MPI_COMM_WORLD);
  MPI_Barrier(MPI_COMM_WORLD);
}


void gemm(ScatterGatherInfo *sgInfo, double *blockA, double *blockB, double *blockC, double *blockT)
{
  int n = sgInfo->blockSize, N = sgInfo->gridSize;
  // Create column and row communicators
  MPI_Comm gridComm, colComm, rowComm;
  MPI_Cart_create(MPI_COMM_WORLD, 2, (int[]){N, N}, (int[]){1, 1}, 0, &gridComm);
  MPI_Cart_sub(gridComm, (int[]){1, 0}, &colComm);
  MPI_Cart_sub(gridComm, (int[]){0, 1}, &rowComm);

  // Get row and column indices + indices of column neighbours
  int i, j, src, dst, glob;
  MPI_Comm_rank(colComm, &i);
  MPI_Comm_rank(rowComm, &j);
  MPI_Cart_shift(colComm, 0, 1, &src, &dst);
  MPI_Comm_rank(MPI_COMM_WORLD, &glob);

  // Cij = sum(k=0àN-1) A[i,(i+k)%N] B[(i+k)%N,j]
  for (int k = 0; k < N; k++)
  {

    // Broadcast A[i,(i+k)%N]
    cblas_dcopy(n * n, blockA, 1, blockT, 1);
    MPI_Bcast(blockT, n * n, MPI_DOUBLE, (i + k) % N, rowComm);
    MPI_Barrier(MPI_COMM_WORLD);

    // Add to A[i,(i+k)%N] B[(i+k)%N,j] to Cij
    cblas_dgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans, n, n, n,
                1, blockT, n, blockB, n, 1, blockC, n);

    // Shift Bs to prepare B[(i+k)%N,j] for next iteration
    MPI_Sendrecv_replace(blockB, n * n, MPI_DOUBLE, dst, 0, src, 0, colComm, MPI_STATUS_IGNORE);

  }
}

int main(int argc, char **argv)
{
  MPI_Init(NULL, NULL);
  int rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  double *A = NULL,
         *B = NULL,
         *blockA = NULL,
         *blockB = NULL,
         *blockC = NULL,
         *blockT = NULL;

  struct timeval start, stop;
  double scatterTime = 0, gemmTime = 0, gatherTime = 0; 

  int blockSize = 0;
  int gridSize;

  parse(argc, argv, &A, &B, &blockSize, &gridSize);

  if (blockSize != 0) {

    blockA = (double *) malloc(blockSize * blockSize * sizeof(double));
    blockB = (double *) malloc(blockSize * blockSize * sizeof(double));
    blockC = (double *) malloc(blockSize * blockSize * sizeof(double));
    blockT = (double *) malloc(blockSize * blockSize * sizeof(double));

    ScatterGatherInfo *sgInfo = initScatterGatherInfo(blockSize, gridSize);


    if (rank == 0)
    {
      gettimeofday(&start, NULL);
    }
    scatter(sgInfo, A, B, blockA, blockB);
    if (rank == 0)
    {
      gettimeofday(&stop, NULL);
      scatterTime = stop.tv_sec - start.tv_sec + 0.000001 * (stop.tv_usec - start.tv_usec);
    }


    if (rank == 0)
    {
      gettimeofday(&start, NULL);
    }
    gemm(sgInfo, blockA, blockB, blockC, blockT);
    if (rank == 0)
    {
      gettimeofday(&stop, NULL);
      gemmTime = stop.tv_sec - start.tv_sec + 0.000001 * (stop.tv_usec - start.tv_usec);
    }


    if (rank == 0) 
    {
      gettimeofday(&start, NULL);
    }
    gather(sgInfo, A, blockC);
    if (rank == 0)
    {
      gettimeofday(&stop, NULL);
      gatherTime = stop.tv_sec - start.tv_sec + 0.000001 * (stop.tv_usec - start.tv_usec);
    }

    freeScatterGatherInfo(sgInfo);

    if (argc >= 4) 
      writeResult(gridSize*blockSize, A, B, argv[3]);

    free(blockA);
    free(blockB);
    free(blockC);
    free(blockT);
  }
  if (rank == 0) 
    printf("%lf %lf %lf\n", scatterTime, gemmTime, gatherTime);

  MPI_Finalize();
  return EXIT_SUCCESS;
}
