#include <math.h>
#include <mpi.h>
#include <sys/time.h>
#include <stdlib.h>

#include "Cell.h"
#include "Quadtree.h"


#define FAR_FIELD_LIMIT 0.707

int main(int argc, char const *argv[])
{
	MPI_Init(NULL, NULL);
	int rank, size;
	double buildTime, multipoleTime, interactionTime;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	if (argc != 3)
	{
		if (rank == 0)
			printf("Usage: %s nbParticles treeHeight\n", argv[0]);
		return EXIT_FAILURE;
	}

	int nbParticles = atoi(argv[1]);
	int height = atoi(argv[2]);
	int nbCells = powl(4, height-1);

	if (nbCells % size != 0)
	{
		if (rank == 0)
			printf("The number of cells (4^(height-1)) must be a multiple of the number of nodes");
		return EXIT_FAILURE;		
	}

	Quadtree qt;
	struct timeval start, stop;

	if (rank == 0)
	{
		printf("# Benching barnes-hut like method, w/ work on cells distributed accross nodes, done in parallel, with vectorized operators\n"
			   "%d particles, tree of height %d\n", nbParticles, height);
		gettimeofday(&start, NULL);
	}

	srand(42);
	initQuadtree(&qt, height, nbParticles, nbParticles, 1e30, 1e32, 0, 1e17, 0, 1e17);
	MPI_Barrier(MPI_COMM_WORLD);

	if (rank == 0)
	{
		gettimeofday(&stop, NULL);
		buildTime = stop.tv_sec - start.tv_sec + 0.000001 * (stop.tv_usec - start.tv_usec);
		gettimeofday(&start, NULL);
	}

	computeMultipoles(&qt);
	MPI_Barrier(MPI_COMM_WORLD);

	if (rank == 0)
	{
		gettimeofday(&stop, NULL);
		multipoleTime = stop.tv_sec - start.tv_sec + 0.000001 * (stop.tv_usec - start.tv_usec);
	  	gettimeofday(&start, NULL);
	}

	computeForcesDistributed(&qt, FAR_FIELD_LIMIT);	

  	if (rank == 0)
	{
		gettimeofday(&stop, NULL);
  		interactionTime = stop.tv_sec - start.tv_sec + 0.000001 * (stop.tv_usec - start.tv_usec);
		printf("#Nb of particles, Quadtree height, Quadtree building time, multipole computation time, interaction computation time (seconds)\n"
			   "%d %d %e %e %e\n\n", nbParticles, height, buildTime, multipoleTime, interactionTime);
  	}

	freeQuadtree(&qt);

	MPI_Finalize();
	return EXIT_SUCCESS;
}