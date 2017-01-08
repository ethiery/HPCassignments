#ifndef DISTRIBUTED_DATA
#define DISTRIBUTED_DATA

#include <mpi.h>


typedef struct DistributedMatrix
{
	int M, N; 		// Global size of the matrix
	int localN; 	// Width of the local matrix
	int blockSize;
	double *buff;	// Matrix of size M x localN

	// For later scattering
	MPI_Datatype sendColBlocksType1;
	MPI_Datatype recvColBlocksType1;
	MPI_Datatype sendColBlocksType2;
	MPI_Datatype recvColBlocksType2;
} DistributedMatrix;

/**
 * Scatters an MxN mat among processor in column blocks, in a 
 * snake layout
 * Exemple for 4 procs : 0 1 2 3 3 2 1 0 0 1 2 3
 *
 * N must be a multiple of blockSize * nbProcs 
 *
 * Done in 2 scatters, sending first : 0 1 2 3 - - - - 0 1 2 3 (sendColBlocksType1)
 * then - - - - 3 2 1 0 - - - - (sendColBlocksType2)
 */ 
DistributedMatrix *scatterMatrix(int M, int N, double *mat, int ld, int blockSize, int root);

/**
 * Gathers an MxN mat previously scattered among processor in column blocks, in a 
 * snake layout by scatterMatrix(...)
 */ 
void gatherMatrix(double *mat, DistributedMatrix *dmat, int root);

#endif