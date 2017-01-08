#include "utils.h"

#include <assert.h>
#include <math.h>
#include <mkl.h>
#include <stdio.h>
#include <string.h>

/** Command line parsing */

const int nbDgetrfFuncs = 4;
const generic_dgetrf dgetrfFuncs[] = { dgetrf_MKL, dgetrf_customBlas2Nopiv, dgetrf_customBlas3Nopiv, dgetrf_customDistributedNopiv };
const char* dgetrfFuncNames[] = { "dgetrf_MKL", "dgetrf2_nopiv", "dgetrf_nopiv", "pdgetrf_nopiv" }; 

void printUsage(const char *execName)
{
	fprintf(stderr, "Usage:\n%s dgetrfFuncName M N minCoeff maxCoeff seed coarseBlockSize fineBlockSize\n"
    			"with dgetrfFuncName in { dgetrf_MKL, dgetrf2_nopiv, dgetrf_nopiv, pdgetrf_nopiv}\n", execName);
}

/**
 * Parses command line arguments and initialize parameters
 * Returns 0 in case of success, -1 else;
 */
int parse(int argc, char const *argv[], int *M, int *N, double *min, double *max, 
		   int *seed, int *coarseBlockSize, int *fineBlockSize, generic_dgetrf *func, char **name)
{
    if (argc != 9)
    {
    	printUsage(argv[0]);
    	return -1;
    }

    *name = (char *)argv[1];
    *func = NULL;
    for (int i = 0; i < nbDgetrfFuncs; i++)
    {
    	if (strcmp(dgetrfFuncNames[i], *name) == 0)
    	{
    		*func = dgetrfFuncs[i];
    		break;	
    	}
    }

    if (*func == NULL)
    {
    	printUsage(argv[0]);
    	return -1;
    }

    *M = atoi(argv[2]);
    *N = atoi(argv[3]);
    *min = atoi(argv[4]);
    *max = atoi(argv[5]);
    *seed = atoi(argv[6]);
    *coarseBlockSize = atoi(argv[7]);
    *fineBlockSize = atoi(argv[8]);

  	return 0;
}


/** Test data initialization */


/** Allocates and initializes a MxN copy of leading dimension M of mat
 *  mat must be a MxN matrix of leading dimension ld
 */
double *copyMatrix(int M, int N, double *mat, int ld)
{
	double *copy = (double *) malloc(M * N * sizeof(double));
	
	for (int i = 0; i < N; i++)
		memcpy(copy + (i*M), mat + (i*ld), M * sizeof(double));

	return copy;
}

/**
 * Allocates and initializes a MxN matrix with doubles between min and max
 */
double *randomMatrix(int M, int N, double min, double max)
{
	double *mat = (double *) malloc(M * N * sizeof(double));

	for (int i = 0; i < M; i++)
		for (int j = 0; j < N; j++)
			mat[j*M + i] = (rand()/(float)(RAND_MAX)) * (max - min) + min;

	return mat;
}


/**
 * Allocates and initializes a system of N linear equations:
 *
 *     1 1 0 0 ... 0                   1                2
 *     1 0 2 0 ... 0                   1                3
 *     1 0 0 3 ... 0                   1                4
 * A = .       .   .       times  X =  .     equals B = .
 *     1 0 0 0 ... n-1                 1                N
 *     1 0 0 0 ... 0                   1                1
 *
 * if Acopy, Bcopy and Xcopy are not null, then they are initialized
 * with copies of A, B, and X 
 *
 */
void initDebugSystem(int N, double **A, double **Acopy, double **B, double **Bcopy, double **X, double **Xcopy)
{
	if (A != NULL)
	{
		*A = (double *) calloc(N * N, sizeof(double));
		for (int i = 0; i < N; i++)
			(*A)[i] = 1;
		for (int i = 0; i < N-1; i++)
			(*A)[(i+1)*N + i] = i+1;

		if (Acopy != NULL)
			*Acopy = copyMatrix(N, N, *A, N);
	}

	if (B != NULL)
	{
		*B = (double *) malloc(N * sizeof(double));
		for (int i = 0; i < N-1; i++)
			(*B)[i] = 2 + i;
		(*B)[N-1] = 1;

		if (Bcopy != NULL)
			*Bcopy = copyMatrix(N, 1, *B, N);
	}

	if (X != NULL)
	{
		*X = (double *) malloc(N * sizeof(double));
		for (int i = 0; i < N; i++)
			(*X)[i] = 1;

		if (Xcopy != NULL)
			*Xcopy = copyMatrix(N, 1, *X, N);
	}
}


/**
 * Allocates and initializes a system of N random linear equations (with coefficients
 * between min and max):
 *
 * if Acopy and Bcopy are not null, then they are initialized
 * with copies of A, and B 
 *
 */
void initRandomTestSystem(int N, double min, double max, double **A, double **Acopy, double **B, double **Bcopy)
{
	if (A != NULL)
	{
		*A = randomMatrix(N, N, min, max);

		if (Acopy != NULL)
			*Acopy = copyMatrix(N, N, *A, N);
	}

	if (B != NULL)
	{
		*B = randomMatrix(N, 1, min, max);

		if (Bcopy != NULL)
			*Bcopy = copyMatrix(N, 1, *B, N);
	}
}

/**
 * Allocates and initializes a M*min(M, N) lower triangular matrix,
 * with 1 on its diagonal, and the same values as the matrix LU on its 
 * lower part. 
 */
double *cpyL(int M, int N, double *LU, int ld)
{
	int nbRows = M;
	int nbCols = (M < N) ? M : N;
	double *L = (double *) calloc(nbRows * nbCols, sizeof(double));

	for (int i = 0; i < nbRows; i++)
		for (int j = 0; j < i && j < nbCols; j++)
			L[j * nbRows + i] = LU[j * ld + i];

	for (int i = 0; i < nbRows && i < nbCols; i++)
		L[i * nbRows + i] = 1;

	return L;
}

/**
 * Allocates and initializes a min(M, N)*N upper triangular matrix,
 * with the same values as the matrix LU on its non zero part
 */
double *cpyU(int M, int N, double *LU, int ld)
{
	int nbRows = (M < N) ? M : N;
	int nbCols = N;
	
	double *U = (double *) calloc(nbRows * nbCols, sizeof(double));

	for (int i = 0; i < nbRows; i++)
		for (int j = i; j < nbCols; j++)
			U[j * nbRows + i] = LU[j * ld + i];

	return U;
}


/**
 * Returns the maximal element of |A - LU|
 */
double maxElementwiseError(int M, int N, 
						 double *A, int lda, 
						 double *LU, int ldlu)
{
	double *L = cpyL(M, N, LU, ldlu);
	double *U = cpyU(M, N, LU, ldlu);
	double *AminusLU = (double *) malloc(M * N * sizeof(double));
	for (int i = 0; i < N; i++)
		cblas_dcopy(M, A + lda*i, 1, AminusLU + M*i, 1);

	int K = (M < N) ? M : N;
	cblas_dgemm(CblasColMajor, CblasNoTrans, CblasNoTrans, 
				M, N, K, 
				1, L, M, U, K, 
				-1, AminusLU, M);

	double max = fabs(AminusLU[cblas_idamax(M*N, AminusLU, 1)]);

	free(L); free(U); free(AminusLU);
	return max;
}

/**
 * Computes the normwise backward error on the solution X of a linear system A*X = B
 * Defined as:
 * 
 * min { eps : (A + dA)x = b + db, ||dA|| <= eps ||A||, ||dB|| <= eps ||b|| }
 *        
 *        = ||Ax - b|| / (||A||*||x|| + ||b||)
 * 
 * where norms are 2-norms
 *
 * A is a general NxN matrix of leading dimension lda
 * X and B are vectors of size N
 *
 * Note: this implementation is not efficient at all, as it computes
 * the full spectrum of trans(A)*A
 */ 
double normwiseBackwardError(int N, double *A, int lda, double *X, double *B)
{
	// R = B - AX
	double *R = (double *) malloc(N * sizeof(double));
	cblas_dcopy(N, B, 1, R, 1);
	cblas_dgemv(CblasColMajor, CblasNoTrans, N, N, -1, A, lda, X, 1, 1, R, 1);

	double normX = cblas_dasum(N, X, 1);
	double normB = cblas_dnrm2(N, B, 1);
	double normR = cblas_dnrm2(N, R, 1);

	// ATA = trans(A) * A
	double *ATA = (double *) malloc(N * N * sizeof(double));
	cblas_dgemm(CblasColMajor, CblasTrans, CblasNoTrans, N, N, N, 1, A, lda, A, lda, 0, ATA, N);

	// normA = spectral norm of A = sqrt(max eigenvalue of ATA)
	double *eigval = (double *) malloc(N * sizeof(double));
	assert(LAPACKE_dsyev(LAPACK_COL_MAJOR, 'N', 'U', N, ATA, lda, eigval) == 0);
	double normA = sqrt(eigval[N-1]);

	double backwardError = normR / (normA * normX + normB);
	// double backwardError = normTmp / normB;
	free(ATA);
	free(eigval);
	free(R);
	return backwardError;
}

/**
 * Computes the partial normwise backward error on the solution X of a linear system A*X = B
 * Defined as:
 * 
 * min { eps : Ax = b + db, ||dB|| <= eps ||b|| }
 *
 *        = ||Ax - b|| / ||b||
 * 
 * where norms are 2-norms
 *
 * A is a general NxN matrix of leading dimension lda
 * X and B are vectors of size N
 */ 
double partialNormwiseBackwardError(int N, double *A, int lda, double *X, double *B)
{
	// R = B - AX
	double *R = (double *) malloc(N * sizeof(double));
	cblas_dcopy(N, B, 1, R, 1);
	cblas_dgemv(CblasColMajor, CblasNoTrans, N, N, -1, A, lda, X, 1, 1, R, 1);

	double normB = cblas_dnrm2(N, B, 1);
	double normR = cblas_dnrm2(N, R, 1);

	double backwardError = normR / normB;
	free(R);
	return backwardError;
}


/** Debug helper methods */

void printMatrix(int M, int N, double *mat, int ld)
{
	for (int i = 0; i < M; i++)
	{
		for (int j = 0; j < N; j++)
		{
			printf("%.1e ", mat[j * ld + i]);
		}
		printf("\n");
	}
}


