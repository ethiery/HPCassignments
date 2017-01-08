#include <sys/time.h>
#include <stdlib.h>

#include "Cell.h"
#include "Quadtree.h"

#define FAR_FIELD_LIMIT 0.707

int main(int argc, char const *argv[])
{
  if (argc != 2)
  {
    printf("Usage: %s nbParticles\n", argv[0]);
    return EXIT_FAILURE;
  }

  int nbParticles = atoi(argv[1]);
  Quadtree qt;
  Cell cMerged;
  struct timeval start, stop;

  printf("# Benching naive quadratic method, sequential, not distributed");

  srand(42);
  gettimeofday(&start, NULL);
  initQuadtree(&qt, 4, nbParticles, nbParticles, 1e30, 1e32, 0, 1e17, 0, 1e17);
  mergeCell(&cMerged, qt.nbCells, qt.cells);
  gettimeofday(&stop, NULL);
  double buildTime = stop.tv_sec - start.tv_sec + 0.000001 * (stop.tv_usec - start.tv_usec);

  gettimeofday(&start, NULL);
  P2P_inRef(&cMerged);
  gettimeofday(&stop, NULL);
  double interactionTime = stop.tv_sec - start.tv_sec + 0.000001 * (stop.tv_usec - start.tv_usec);


  printf("# nbParticles, 0, Particles generation time, 0 (multipole computation time), interaction computation time (seconds)\n"
       "%d 0 %e 0 %e\n\n", nbParticles, buildTime, interactionTime);

  freeQuadtree(&qt);
  freeCell(&cMerged);
  return EXIT_SUCCESS;
}