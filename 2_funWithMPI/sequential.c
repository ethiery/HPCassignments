#include "sequential.h"
#include "util.h"

#include <assert.h>
#include <float.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>

#define EPS 1e-10


/**
 * Assumes that inputFileName is a path to a valid input file
 */
void seqSimulation(char* inputFileName, int nbSteps, double r,
                   char* outputDir, int outputPeriod)
{
  FILE *inputFile = fopen(inputFileName, "r");
  long nbParticles = parseNbParticles(inputFile);
  Particle_Private *p = parseInitialConditions(inputFile, 0, nbParticles);
  fclose(inputFile);

  FILE *outputFile;
  int i, j, step, nextSnapshot = outputPeriod;
  double timeStep, minTimeStep;

  for (step = 0; step < nbSteps; step++)
  {
    minTimeStep = DBL_MAX;

    // thanks to newston's second law, we only need to compute half the
    // interactions
    for (i = 0; i < nbParticles; i++)
    {
      for (j = i+1; j < nbParticles; j++)
      {
          addInteractions(p+i, p+j);
      }
      // Computes an upper bound on the time step
      // for this particle to travel less than 10% of the distance between
      // this particle and its closest neighbour
      timeStep = dtUpperBound(p+i, r);
      minTimeStep = timeStep < minTimeStep ? timeStep : minTimeStep;
    }

    // We now know the total force applied on each particle, and a
    // timestep so that each of the particles travel less than 10% to their
    // closest neighbour.
    // We can apply the step
    for (i = 0; i < nbParticles; i++)
    {
      applyForces(p+i, minTimeStep);
    }

    // Writes current state to file if an output directory is specified
    if (outputDir != NULL && step+1 == nextSnapshot)
    {
      outputFile = initSnapshotFile(outputDir, step, nbParticles);
      dumpSnapshot(outputFile, nbParticles, p);
      fclose(outputFile);
      nextSnapshot += outputPeriod;
    }
  }
  free(p);
}
