#include <assert.h>
#include <math.h>
#include <mkl.h>
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "genericWrapper.h"
#include "utils.h"
#include "LU.h"

const int DIM1 = 2000;
const int DIM2 = 1000;
const int MIN = -5;
const int MAX = 5;
const int COARSE_BS = 50;
const int FINE_BS = 10;
const int SEED = 42;


void benchDgetrf(generic_dgetrf func, const char* name, int M, int N, double min, double max, int seed, 
				 int coarseBlockSize, int fineBlockSize, int root)
{
	int rank; MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  	double *A = NULL, *LU = NULL;
  	lapack_int *ipiv = NULL;
  	int minMN = (M < N) ? M : N;

	if (rank == root)
  	{
		srand(seed);
		A = randomMatrix(M, N, min, max);
		LU = copyMatrix(M, N, A, M);
		ipiv = (lapack_int *) malloc(minMN * sizeof(lapack_int));
		for (int i = 0; i < minMN; i++) ipiv[i] = i+1;
	}

	double duration = func(M, N, LU, M, ipiv, coarseBlockSize, fineBlockSize, root);

	MPI_Barrier(MPI_COMM_WORLD);

	if (rank == root)
	{
		printf("%f\n", duration);
		free(A); free(LU);	
	}
}



int main(int argc, char const *argv[])
{
	MPI_Init(NULL, NULL);

	int rank; MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	int M = 0, N = 0, seed = 0, coarseBlockSize = 0, fineBlockSize = 0, root = 0;
	double min = 0, max = 0;
	generic_dgetrf func = NULL;
	char *name;

	if (parse(argc, argv, &M, &N, &min, &max, &seed, &coarseBlockSize, &fineBlockSize, &func, &name) != 0)
		return EXIT_FAILURE;

	if (rank == root)
		printf("# Time in s for LU facto on random [%.1e, %.1e]^%d x [%.1e, %.1e]^%d matrix (seed = %d, block sizes = %d, %d) with %s\n",
			   min, max, M, min, max, N, seed, coarseBlockSize, fineBlockSize, name);

	benchDgetrf(func, name, M, N, min, max, seed, coarseBlockSize, fineBlockSize, root); 

	mkl_thread_free_buffers();

	MPI_Finalize();
	return EXIT_SUCCESS;
}