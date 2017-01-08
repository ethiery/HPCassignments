#include "parallel.h"
#include "Particle.h"
#include "util.h"

#include <assert.h>
#include <float.h>
#include <math.h>
#include <mpi.h>
#include <stdlib.h>
#include <string.h>

inline void swap(Particle_Shared **p1, Particle_Shared **p2)
{
  Particle_Shared *SWAP = *p1;
  *p1 = *p2;
  *p2 = SWAP;
}

/**
 * Assumes that inputFileName is a path to a valid input file
 */
void parallelSimulation(char* inputFileName, int nbSteps, double r,
                   char* outputDir, int outputPeriod)
{
  // MPI initialization
  int rank, size, prev, next;
  MPI_Request sendReq, recvReq;
  MPI_Status status;
  MPI_Init(NULL, NULL);
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  prev = (rank - 1) % size;
  next = (rank + 1) % size;
  register_MPI_PARTICLE();

  // Initialize private particles, and particle buffers
  FILE *inputFile = fopen(inputFileName, "r");
  long nbParticles = parseNbParticles(inputFile);
  assert(nbParticles % size == 0);
  nbParticles /= size;
  Particle_Private *myParticles = parseInitialConditions(inputFile, rank*nbParticles, nbParticles);
  Particle_Shared *cmptBuff = (Particle_Shared *) malloc(nbParticles * sizeof(Particle_Shared));
  Particle_Shared *recvBuff = (Particle_Shared *) malloc(nbParticles * sizeof(Particle_Shared));
  for (int i = 0; i < nbParticles; i++)
  {
    cmptBuff[i].m = myParticles[i].m;
    cmptBuff[i].x = myParticles[i].x;
    cmptBuff[i].y = myParticles[i].y;
  }
  fclose(inputFile);



  int nextSnapshot = outputPeriod;
  for (int step = 0; step < nbSteps; step++)
  {
    for (int k = 0; k < size; k++)
    {
      MPI_Isend(cmptBuff, nbParticles, MPI_PARTICLE, next, 0, MPI_COMM_WORLD, &sendReq);
      MPI_Irecv(recvBuff, nbParticles, MPI_PARTICLE, prev, 0, MPI_COMM_WORLD, &recvReq);
      for (int i = 0; i < nbParticles; i++)
        for (int j = 0; j < nbParticles; j++)
        {
            addAction(cmptBuff+j, myParticles+i);
        }
      MPI_Wait(&sendReq, &status);
      MPI_Wait(&recvReq, &status);
      swap(&cmptBuff, &recvBuff);
    }

    // Compute the maximum time step for the particles of the process
    double timeStep, minTimeStep = DBL_MAX;
    for (int i = 0; i < nbParticles; i++)
    {
      // printf("force: %f %f\n", pp[i].fx, pp[i].fy);
      timeStep = dtUpperBound(myParticles+i, r);
      minTimeStep = timeStep < minTimeStep ? timeStep : minTimeStep;
    }
    // Compute the global maximal time step
    timeStep = minTimeStep;
    MPI_Allreduce(&timeStep, &minTimeStep, 1, MPI_DOUBLE, MPI_MIN, MPI_COMM_WORLD);

    // Apply forces on the particles of the process, and update sharing buffers
    for (int i = 0; i < nbParticles; i++)
    {
      applyForces(myParticles+i, minTimeStep);
      cmptBuff[i].m = myParticles[i].m;
      cmptBuff[i].x = myParticles[i].x;
      cmptBuff[i].y = myParticles[i].y;
    }

    // Writes current state to file if an output directory is specified
    if (outputDir != NULL && step+1 == nextSnapshot)
    {
      int token; // token sent around the ring to sync the writting
      FILE *outputFile;
      if (rank == 0)
      {
        outputFile = initSnapshotFile(outputDir, step, nbParticles*size);
        dumpSnapshot(outputFile, nbParticles, myParticles);
        MPI_Send(&token, 1, MPI_INT, next, 0, MPI_COMM_WORLD);
      }
      else
      {
        MPI_Recv(&token, 1, MPI_INT, prev, 0, MPI_COMM_WORLD, &status);
        outputFile = getSnapshotFile(outputDir, step);
        dumpSnapshot(outputFile, nbParticles, myParticles);
        if (next != 0)
          MPI_Send(&token, 1, MPI_INT, next, 0, MPI_COMM_WORLD);
      }
      fclose(outputFile);
      nextSnapshot += outputPeriod;
    }
  }
  free(myParticles);
  free(cmptBuff);
  free(recvBuff);
  MPI_Finalize();
}
