#include <assert.h>
#include <malloc.h>

#include "distributedData.h"

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
DistributedMatrix *scatterMatrix(int M, int N, double *mat, int ld, int blockSize, int root)
{
  int rank, nbProcs;
  MPI_Comm_size(MPI_COMM_WORLD, &nbProcs);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  int nbColBlocks = N / (blockSize * nbProcs);
  assert(nbColBlocks * nbProcs * blockSize == N);

  DistributedMatrix *dmat = (DistributedMatrix *) malloc(sizeof(DistributedMatrix));
  dmat->M = M;
  dmat->N = N;
  dmat->localN = nbColBlocks * blockSize;
  dmat->blockSize = blockSize;

  // sendColBlockType -> blockSize consecutive columns of M doubles, with leading dimension ld
  MPI_Datatype sendColBlockType, tmpType;
  MPI_Type_vector(blockSize, M, ld, MPI_DOUBLE, &tmpType);
  MPI_Type_create_resized(tmpType, 0, blockSize*ld*sizeof(double), &sendColBlockType);

	// sendColBlocksType1 & sendColBlocksType1 -> 1 sendColBlockType every N, where N is the number of process
	// Only the number of sendColBlockType can vary from 1 
  int nbColBlocks1 = (nbColBlocks + 1) / 2;
  MPI_Type_vector(nbColBlocks1, 1, 2*nbProcs, sendColBlockType, &tmpType);
  MPI_Type_create_resized(tmpType, 0, blockSize*ld*sizeof(double), &(dmat->sendColBlocksType1));
  MPI_Type_commit(&(dmat->sendColBlocksType1));

  int nbColBlocks2 = nbColBlocks - nbColBlocks1;
  MPI_Type_vector(nbColBlocks2, 1, 2*nbProcs, sendColBlockType, &tmpType);
  MPI_Type_create_resized(tmpType, 0, blockSize*ld*sizeof(double), &(dmat->sendColBlocksType2));
  MPI_Type_commit(&(dmat->sendColBlocksType2));

  // recvColBlocksType1 & recvColBlocksType2 -> 1 column of M doubles over 2, with leading dimension M
	// The number of colums can vary from 1 between the 2 types 
  MPI_Type_vector(nbColBlocks1, M*blockSize, 2*M*blockSize, MPI_DOUBLE, &(dmat->recvColBlocksType1));
  MPI_Type_commit(&(dmat->recvColBlocksType1));
  MPI_Type_vector(nbColBlocks2, M*blockSize, 2*M*blockSize, MPI_DOUBLE, &(dmat->recvColBlocksType2));
  MPI_Type_commit(&(dmat->recvColBlocksType2));

  dmat->buff = (double *) calloc(nbColBlocks * M * blockSize, sizeof(double));

  MPI_Scatter(mat, 1, dmat->sendColBlocksType1, 
              dmat->buff, 1, dmat->recvColBlocksType1, 
              root, MPI_COMM_WORLD);

  int sendcounts[nbProcs];
  int displs[nbProcs];
  for (int i = 0; i < nbProcs; i++)
  {
    sendcounts[i] = 1;
    displs[i] = 2*nbProcs - (i+1);
  }

  MPI_Scatterv(mat, sendcounts, displs, dmat->sendColBlocksType2,
              dmat->buff + M*blockSize, 1, dmat->recvColBlocksType2,
              root, MPI_COMM_WORLD);

  return dmat;
}

/**
 * Gathers an MxN mat previously scattered among processor in column blocks, in a 
 * snake layout by scatterMatrix(...)
 */ 
void gatherMatrix(double *mat, DistributedMatrix *dmat, int root)
{
  int nbProcs;
  MPI_Comm_size(MPI_COMM_WORLD, &nbProcs);

  MPI_Gather(dmat->buff, 1, dmat->recvColBlocksType1,
             mat, 1, dmat->sendColBlocksType1,
             root, MPI_COMM_WORLD);

  int recvcounts[nbProcs];
  int displs[nbProcs];
  for (int i = 0; i < nbProcs; i++)
  {
    recvcounts[i] = 1;
    displs[i] = 2*nbProcs - (i+1);
  }

  MPI_Gatherv(dmat->buff + dmat->M*dmat->blockSize, 1, dmat->recvColBlocksType2,
              mat, recvcounts, displs, dmat->sendColBlocksType2,
              root, MPI_COMM_WORLD);

  free(dmat->buff);
  MPI_Type_free(&(dmat->recvColBlocksType2));
  MPI_Type_free(&(dmat->recvColBlocksType1));
  MPI_Type_free(&(dmat->sendColBlocksType2));
  MPI_Type_free(&(dmat->sendColBlocksType1));
  free(dmat);
}