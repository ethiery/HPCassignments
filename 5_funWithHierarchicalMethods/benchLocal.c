#include "sys/time.h"
#include <stdlib.h>

#include "Cell.h"
#include "Quadtree.h"

#define FAR_FIELD_LIMIT 0.707

int main(int argc, char const *argv[])
{
	if (argc != 3)
	{
		printf("Usage: %s nbParticles treeHeight\n", argv[0]);
		return EXIT_FAILURE;
	}

	int nbParticles = atoi(argv[1]);
	int height = atoi(argv[2]);
	Quadtree qt;
	struct timeval start, stop;

	printf("# Benching barnes-hut like method, cells in parallel with vectorized operators, but not distributed\n"
		   "%d particles, tree of height %d\n", nbParticles, height);

	srand(42);
	gettimeofday(&start, NULL);
	initQuadtree(&qt, height, nbParticles, nbParticles, 1e30, 1e32, 0, 1e17, 0, 1e17);
	gettimeofday(&stop, NULL);
	double buildTime = stop.tv_sec - start.tv_sec + 0.000001 * (stop.tv_usec - start.tv_usec);

	gettimeofday(&start, NULL);
	computeMultipoles(&qt);
	gettimeofday(&stop, NULL);
	double multipoleTime = stop.tv_sec - start.tv_sec + 0.000001 * (stop.tv_usec - start.tv_usec);

  	gettimeofday(&start, NULL);
	computeForces(&qt, FAR_FIELD_LIMIT);	
  	gettimeofday(&stop, NULL);
  	double interactionTime = stop.tv_sec - start.tv_sec + 0.000001 * (stop.tv_usec - start.tv_usec);

	printf("#Nb of particles, Quadtree height, Quadtree building time, multipole computation time, interaction computation time (seconds)\n"
		   "%d %d %e %e %e\n\n", nbParticles, height, buildTime, multipoleTime, interactionTime);

	freeQuadtree(&qt);
	return EXIT_SUCCESS;
}