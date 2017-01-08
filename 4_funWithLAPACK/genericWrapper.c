#include "genericWrapper.h"

#include <mpi.h>
#include <sys/time.h>

#include "LU.h"
#include "utils.h"
#include "distributedData.h"


double dgetrf_MKL(int M, int N, double *LU, double ld, lapack_int *ipiv, int coarseBlockSize, int fineBlockSize, int root)
{
	int rank; MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	struct timeval start, stop;

    gettimeofday(&start, NULL);

	if (rank == root)
		LAPACKE_dgetrf(LAPACK_COL_MAJOR, M, N, LU, M, ipiv);

	MPI_Barrier(MPI_COMM_WORLD);
    gettimeofday(&stop, NULL);

	return stop.tv_sec - start.tv_sec + 0.000001 * (stop.tv_usec - start.tv_usec);
}

double dgetrf_customBlas2Nopiv(int M, int N, double *LU, double ld, lapack_int *ipiv, int coarseBlockSize, int fineBlockSize, int root)
{
	int rank; MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	struct timeval start, stop;

    gettimeofday(&start, NULL);

	if (rank == root)
		dgetrf2_nopiv(M, N, LU, M);
	
	MPI_Barrier(MPI_COMM_WORLD);
	gettimeofday(&stop, NULL);

	return stop.tv_sec - start.tv_sec + 0.000001 * (stop.tv_usec - start.tv_usec);
}

double dgetrf_customBlas3Nopiv(int M, int N, double *LU, double ld, lapack_int *ipiv, int coarseBlockSize, int fineBlockSize, int root)
{
	int rank; MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	struct timeval start, stop;

    gettimeofday(&start, NULL);

	if (rank == root)
		dgetrf_nopiv(M, N, LU, M, coarseBlockSize);

	MPI_Barrier(MPI_COMM_WORLD);
	gettimeofday(&stop, NULL);

	return stop.tv_sec - start.tv_sec + 0.000001 * (stop.tv_usec - start.tv_usec);
}

double dgetrf_customDistributedNopiv(int M, int N, double *LU, double ld, lapack_int *ipiv, int coarseBlockSize, int fineBlockSize, int root)
{
	struct timeval start, stop;
	DistributedMatrix *dLU = scatterMatrix(M, N, LU, M, coarseBlockSize, root);

    gettimeofday(&start, NULL);

	pdgetrf_nopiv(dLU, fineBlockSize);
	
	MPI_Barrier(MPI_COMM_WORLD);
	gettimeofday(&stop, NULL);

	gatherMatrix(LU, dLU, root);
	
	return stop.tv_sec - start.tv_sec + 0.000001 * (stop.tv_usec - start.tv_usec);
}