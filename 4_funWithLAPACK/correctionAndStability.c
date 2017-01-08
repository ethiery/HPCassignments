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


void testDgetrf(generic_dgetrf func, const char* name, int M, int N, double min, double max, int seed, 
				int coarseBlockSize, int fineBlockSize, int root)
{
	int rank; MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  	double *A = NULL, *LU = NULL;
  	lapack_int *ipiv = NULL;
  	int minMN = (M < N) ? M : N;

	if (rank == root)
  	{
		printf("# LU facto w/ %s on random [%.1e, %.1e]^%d x [%.1e, %.1e]^%d matrix "
			   "(seed = %d, block sizes = %d, %d)\n", 
				name, min, max, M, min, max, N, seed, coarseBlockSize, fineBlockSize);

		srand(seed);
		A = randomMatrix(M, N, min, max);
		LU = copyMatrix(M, N, A, M);
		ipiv = (lapack_int *) malloc(minMN * sizeof(lapack_int));
		for (int i = 0; i < minMN; i++) ipiv[i] = i+1;
	}

	func(M, N, LU, M, ipiv, coarseBlockSize, fineBlockSize, root);

	if (rank == root)
	{
		double *swap = (double *) malloc(N * sizeof(double));
		for (int i = 0; i < minMN; i++)
		{
			cblas_dcopy(N, A + i, M, swap, 1);
			cblas_dcopy(N, A + (ipiv[i] - 1), M, A + i, M);
			cblas_dcopy(N, swap, 1, A + (ipiv[i] - 1), M);
		}
		free(swap);

		double maxErr = maxElementwiseError(M, N, A, M, LU, M); 
		
		printf("# Maximal elementwise error between A and LxU:\n%e\n\n", maxErr);
		
		free(A); free(LU);	
	}
}

void testDgetrs(generic_dgetrf func, const char* name, int N, double min, double max, int seed, 
				int coarseBlockSize, int fineBlockSize, int root)
{
	int rank; MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	double *A = NULL, *B = NULL, *X = NULL, *LU = NULL;
	lapack_int *ipiv = NULL;

	if (rank == root)
  	{
  		printf("# Solve random system of size %d with coeffs in [%.1e, %.1e], through LU facto w/ %s "
			   "(seed = %d, block sizes = %d, %d)\n", 
				N, min, max, name, seed, coarseBlockSize, fineBlockSize);

  		srand(seed);
		initRandomTestSystem(N, min, max, &A, &LU, &B, &X);
		ipiv = (lapack_int *) malloc(N * N * sizeof(lapack_int));
		for (int i = 0; i < N; i++) ipiv[i] = i+1;
	}

	func(N, N, LU, N, ipiv, coarseBlockSize, fineBlockSize, root);
	
	if (rank == root)
	{
		double *swap = (double *) malloc(N * sizeof(double));
		for (int i = 0; i < N; i++)
		{
			cblas_dcopy(N, A + i, N, swap, 1);
			cblas_dcopy(N, A + (ipiv[i] - 1), N, A + i, N);
			cblas_dcopy(N, swap, 1, A + (ipiv[i] - 1), N);
		}
		free(swap);

		dgetrs2_nopiv(N, LU, N, X);

		printf("Normwise backward error = %e\n", normwiseBackwardError(N, A, N, X, B));
		printf("RHS-only normwise backward error = %e\n\n", partialNormwiseBackwardError(N, A, N, X, B));	

		free(A); free(B); free(X); free(LU);
	}
}

int main(int argc, char const *argv[])
{
	MPI_Init(NULL, NULL);

	testDgetrf(dgetrf_MKL, "dgetrf_MKL", DIM1, DIM2, MIN, MAX, SEED, COARSE_BS, FINE_BS, 0); 
	testDgetrf(dgetrf_customBlas2Nopiv, "dgetrf2_nopiv", DIM1, DIM2, MIN, MAX, SEED, COARSE_BS, FINE_BS, 0); 
	testDgetrf(dgetrf_customBlas3Nopiv, "dgetrf_nopiv", DIM1, DIM2, MIN, MAX, SEED, COARSE_BS, FINE_BS, 0); 
	testDgetrf(dgetrf_customDistributedNopiv, "pdgetrf_nopiv", DIM1, DIM2, MIN, MAX, SEED, COARSE_BS, FINE_BS, 0); 

	testDgetrs(dgetrf_MKL, "dgetrf_MKL", DIM1, MIN, MAX, SEED, COARSE_BS, FINE_BS, 0); 
	testDgetrs(dgetrf_customBlas2Nopiv, "dgetrf2_nopiv", DIM1, MIN, MAX, SEED, COARSE_BS, FINE_BS, 0); 
	testDgetrs(dgetrf_customBlas3Nopiv, "dgetrf_nopiv", DIM1, MIN, MAX, SEED, COARSE_BS, FINE_BS, 0); 
	testDgetrs(dgetrf_customDistributedNopiv, "pdgetrf_nopiv", DIM1, MIN, MAX, SEED, COARSE_BS, FINE_BS, 0); 

	mkl_thread_free_buffers();

	MPI_Finalize();
	return EXIT_SUCCESS;
}