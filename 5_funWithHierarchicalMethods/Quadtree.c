#include "Morton.h"
#include "Quadtree.h"

#include <math.h>
#include <mpi.h>
#include <omp.h>
// Private auxiliary functions prototypes

// Compute the centers of mass of the i-th vertex and its children
static void computeCMrec(Quadtree *qt, int cmNo);

// Public functions

// Initialise the cells of a quadtree of specified height, built on the 2D area [xMin, xMax]*[yMin, yMax]
// containing a total of particles picked randomly in [[nbPartMin, nbPartMax]], 
// with masses picked randomly in [[mMin, mMax]]   
void initQuadtree(Quadtree *qt, int height, int nbPartMin, int nbPartMax, 
				  double mMin, double mMax, double xMin, double xMax, double yMin, double yMax)
{
	qt->height = height;
	qt->xMin = xMin;
	qt->xMax = xMax;
	qt->yMin = yMin;
	qt->yMax = yMax;
	qt->nbCells = powl(4, height-1);
	qt->cells = (Cell *) malloc(qt->nbCells * sizeof(Cell));
	qt->nbMultipoles = (powl(4, height) - 1) / 3;
	qt->multipoles = (Multipole *) malloc(qt->nbMultipoles * sizeof(Multipole));
	qt->firstOuterCM = qt->nbMultipoles - qt->nbCells; 
	
	long dim = powl(2, height-1);
	double dX = (xMax - xMin) / (double)dim;
	double dY = (yMax - yMin) / (double)dim;
	int nbPartPerCellMin = nbPartMin / qt->nbCells;
	int nbPartPerCellMax = nbPartMax / qt->nbCells;

	int cellNo;
	for (int x = 0; x < dim; x++)
		for (int y = 0; y < dim; y++)
		{
			cellNo = xy_to_morton(x, y);
			initCell(qt->cells + cellNo, nbPartPerCellMin, nbPartPerCellMax, mMin, mMax, 
					 x*dX, (x+1)*dX, y*dY, (y+1)*dY);
		}
}

// Release the ressources associated with the specified quadtree
void freeQuadtree(Quadtree *qt)
{
	for (int i = 0; i < qt->nbCells; i++)
		freeCell(qt->cells + i);
	free(qt->cells);
	free(qt->multipoles);
}


// Compute all the centers of mass of the specified quadtree recursively, 
// starting by the lower level ones
void computeMultipoles(Quadtree *qt)
{
	computeCMrec(qt, 0);
}

// Compute the gravitationnal force exerted on each particule of the quadtree
// Note: we consider that a center of mass is in the far field of a cell
// if d/l > farFieldLimit    where d is the distance between the cm and the cell
// and w is the width of the area approximated by the cm
void computeForces(Quadtree *qt, double farFieldLimit)
{
	#pragma omp parallel
	{
		WorkingVecs wv;
		initWorkingVecs(&wv);

		#pragma omp for schedule(dynamic, 1)
		for (int cellNo = 0; cellNo < qt->nbCells; cellNo++)
		{
			int queue[qt->nbMultipoles];
			int head = 0, tail = 0;
			queue[tail++] = 0;

			int cmNo;
			double d, l;
			while (head < tail)
			{
				cmNo = queue[head++];

				d = distance(qt->multipoles + cmNo, qt->cells + cellNo); 
				l = qt->multipoles[cmNo].xMax - qt->multipoles[cmNo].xMin;

				if (d > l / farFieldLimit)
				{
					M2P(qt->multipoles + cmNo, qt->cells + cellNo, &wv);
				}
				else if (cmNo < qt->firstOuterCM)
				{
					queue[tail++] = 4*cmNo+1;
					queue[tail++] = 4*cmNo+2;
					queue[tail++] = 4*cmNo+3;
					queue[tail++] = 4*cmNo+4;
				}
				else if (cmNo != qt->firstOuterCM + cellNo)
				{
					P2P_ext(qt->cells + (cmNo - qt->firstOuterCM), qt->cells + cellNo, &wv);
				}
			}
			P2P_in(qt->cells + cellNo, &wv);
		}
		
		freeWorkingVecs(&wv);
	}
}

// Compute the gravitationnal force exerted on each particule of the quadtree
// Note: we consider that a center of mass is in the far field of a cell
// if d/l > farFieldLimit    where d is the distance between the cm and the cell
// and w is the width of the area approximated by the cm
void computeForcesDistributed(Quadtree *qt, double farFieldLimit)
{
	int rank, size;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	int nbCellsPerNode = qt->nbCells / size;

	#pragma omp parallel
	{
		WorkingVecs wv;
		initWorkingVecs(&wv);

		#pragma omp for schedule(dynamic, 1)
		for (int cellNo = rank*nbCellsPerNode; cellNo < (rank+1 * nbCellsPerNode); cellNo++)
		{
			int queue[qt->nbMultipoles];
			int head = 0, tail = 0;
			queue[tail++] = 0;

			int cmNo;
			double d, l;
			while (head < tail)
			{
				cmNo = queue[head++];

				d = distance(qt->multipoles + cmNo, qt->cells + cellNo); 
				l = qt->multipoles[cmNo].xMax - qt->multipoles[cmNo].xMin;

				if (d > l / farFieldLimit)
				{
					M2P(qt->multipoles + cmNo, qt->cells + cellNo, &wv);
				}
				else if (cmNo < qt->firstOuterCM)
				{
					queue[tail++] = 4*cmNo+1;
					queue[tail++] = 4*cmNo+2;
					queue[tail++] = 4*cmNo+3;
					queue[tail++] = 4*cmNo+4;
				}
				else if (cmNo != qt->firstOuterCM + cellNo)
				{
					P2P_ext(qt->cells + (cmNo - qt->firstOuterCM), qt->cells + cellNo, &wv);
				}
			}
			P2P_in(qt->cells + cellNo, &wv);
		}
		
		freeWorkingVecs(&wv);
	}
}

// Private auxiliary functions


void computeCMrec(Quadtree *qt, int cmNo)
{
	if (cmNo < qt->firstOuterCM)
	{
		computeCMrec(qt, 4*cmNo+1);
		computeCMrec(qt, 4*cmNo+2);
		computeCMrec(qt, 4*cmNo+3);
		computeCMrec(qt, 4*cmNo+4);
		M2M(qt->multipoles + cmNo, 4, qt->multipoles + 4*cmNo+1);
	}
	else
	{
		P2M(qt->multipoles + cmNo, qt->cells + (cmNo - qt->firstOuterCM));
	}
}
