#include "LU.h"

#include <assert.h>
#include <mkl.h>
#include <stdio.h>


/**
 * LU factorization without pivoting
 * Using only BLAS 2
 *
 * A is a MxN matrix of leading dimension ld
 */
void dgetrf2_nopiv(int M, int N, double *A, int ld)
{
	// Smallest dimension
	const int K = (M < N) ? M : N;

	for (int k = 0; k < K; k ++)
	{
		// Lik = Aik/Ukk   for i in k+1...m
		cblas_dscal(M - (k+1), 1.0 / A[k * ld + k], A + (k * ld + (k+1)), 1);
		
		// Aij = Aij - Lik * Ukj for i in k+1..m and j in k+1..n
		cblas_dger(CblasColMajor, M - (k+1), N - (k+1), 
				   -1, A + (k * ld + (k+1)), 1, 
				   A + ((k+1) * ld + k), ld, 
				   A + ((k+1) * ld + (k+1)), ld);
	}
}

/**
 * LU factorization without pivoting
 * Blocked version with block of size B using BLAS 3
 * A is a MxN matrix of leading dimension A
 *
 * Note: M and N must be multiples of B 
 */
void dgetrf_nopiv(int M, int N, double *A, int lda, int B)
{
	int MB = M/B;
	int NB = N/B;
	assert(M == MB * B);
	assert(N == NB * B);

	// Smallest dimension
	const int KB = (MB < NB) ? MB : NB;

	for (int k = 0; k < KB; k ++)
	{
		dgetrf2_nopiv(M - k*B, B, &A[k*B * lda + k*B], lda);

		cblas_dtrsm(CblasColMajor, CblasLeft, CblasLower, CblasNoTrans, CblasUnit, 
					B, N - (k+1)*B, 
					1, A + (k*B * lda + k*B), lda, 
					A + ((k+1)*B * lda + k*B), lda);

		cblas_dgemm(CblasColMajor, CblasNoTrans, CblasNoTrans, 
					M - (k+1)*B, N - (k+1)*B, B, 
					-1, &A[k*B * lda + (k+1)*B], lda, &A[(k+1)*B * lda + k*B], lda,
					1, &A[(k+1)*B * lda + (k+1)*B], lda);
	}
}

/**
 * LU factorization without pivoting
 * Distributed version with :
 * - Column blocks of width dA->blockSize scattered among MPI nodes
 * - fine grain blocks of size blockSize, which must be a divider of dA->blockSize
 * 
 * dA must be a NxN matrix scattered among MPI nodes with scatterMatrix()
 */
void pdgetrf_nopiv(DistributedMatrix *dA, int fineBlockSize)
{
    int rank, nbProcs;
  	MPI_Comm_size(MPI_COMM_WORLD, &nbProcs);
  	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  	// Global matrix size
  	int M = dA->M,  N = dA->N,  B = dA->blockSize,  ld = dA->M;
  	int K = (N < M) ? N : M;
  	// Local working matrix, pointer and size
  	double *wA = dA->buff;
  	double wM = M,  wN = dA->localN;
  	// Broadcast buffer
  	double *bcastBuff = (double *) malloc(M * B * sizeof(double));
  	
  	// MPI Type representing a block
 	  MPI_Datatype blockType, tmpType;
  	MPI_Type_vector(B, B, M, MPI_DOUBLE, &tmpType);
  	MPI_Type_create_resized(tmpType, 0, B * sizeof(double), &blockType);
  	MPI_Type_commit(&blockType);

  	// Variable used to keep track of whose turn it is in the snake distribution
  	// of the block columsn
  	int root = 0;
  	int count = 0; // goes from 0 to nbProcs-1 in loop
  	char phase = 'I'; // I for increasing, D for decreasing 

  	for (int k = 0; k < K; k += B)
  	{
  		// Computes whose proc is in charge of the k/B-th bloc
		if (count == nbProcs)
  		{
  			count = 0;
  			phase = (phase == 'I') ? 'D' : 'I';
  		}
  		root = (phase == 'I') ? count : nbProcs - 1 - count;
  		count++;

  		// The proc in charge of the first block column of the global working matrix getrf its first column... 
  		if (rank == root)
  		{
  			dgetrf_nopiv(wM, B, wA, ld, fineBlockSize);
  			LAPACKE_dlacpy(LAPACK_COL_MAJOR, 'W', wM, B, wA, ld, bcastBuff, ld);

  			wA += B * ld;
  			wN -= B;
  		}

  		// .. then broadcasts it
  		MPI_Bcast(bcastBuff, wM / B, blockType, root, MPI_COMM_WORLD);

  		// Each proc uses the first block of the bcastBuff to trsm its part of 
  		// the first block line of the global working matrix
  		cblas_dtrsm(CblasColMajor, CblasLeft, CblasLower, CblasNoTrans, CblasUnit, 
					B, wN, 1, bcastBuff, ld, wA, ld);

  		// Each proc uses the other blocks of the bcastBuff to dgemm its part
  		// the other lines of the global working matrix
  		cblas_dgemm(CblasColMajor, CblasNoTrans, CblasNoTrans, wM - B, wN, B, 
  					-1, bcastBuff + B, ld, wA, ld, 1, wA + B, ld);

  		wA += B;
  		wM -= B;
  	}
}

/**
 * Solves a system of linear equations A*X = B using LU factorization 
 * computed by dgetrf_nopiv or dgetrf2_nopiv, and only BLAS 2 operations
 *
 * where LU is the factorization of a general NxN matrix A of leading dimension ldlu,
 * and B is a vector of size N, overwritten with the solution
 */
void dgetrs2_nopiv(int N, double *LU, int ldlu, double *B)
{
	double sum;

	for (int i = 0; i < N; i++)
	{
		B[i] -= cblas_ddot(i, LU + i, ldlu, B, 1);
	}
	for (int i = N-1; i >= 0; i--)
	{
		sum = cblas_ddot(N - i - 1, 
						 LU + ((i+1)*ldlu + i), ldlu, 
						 B + (i+1), 1);
		B[i] = (B[i] - sum) / LU[ldlu*i + i];
	}
}
