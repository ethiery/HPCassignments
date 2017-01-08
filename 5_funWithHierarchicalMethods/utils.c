#include "utils.h"

#include <assert.h>
#include <float.h>
#include <math.h>
#include <stdlib.h>

// Private

static inline double relativeErr(double val, double base)
{
	return fabs(val - base) / fabs(base);
}

static int cmpDbl(const void *p1, const void *p2)
{
	if (*(double*)p1 > *(double*)p2) 
		return 1;
	else if (*(double*)p1 < *(double*)p2) 
		return -1;
	else 
		return 0;
}

// Public

double min(double a, double b)
{
	return (a < b) ? a : b;
}

double max(double a, double b)
{
	return (a > b) ? a : b;
}

double dist(double x1, double y1, double x2, double y2)
{
	return sqrt((x1-x2)*(x1-x2) + (y1-y2)*(y1-y2));
}

extern double randDouble(double minVal, double maxVal)
{
	return minVal + ((float)rand() / (float)(RAND_MAX)) * (maxVal - minVal);
}


// Compute the relative errors between forces applied on the particles of cell "cMerged"
// And particles in the array of cell "cells" (in order).
// Sets the min, max, 1st, 2nd and 3rd quartiles
// relative erros are d(fx)/fx, and d(fy)/fy
void computeRelativeErrors(Cell *cMerged, int nbCells, Cell *cells, 
						   double *minimum, double *maximum, double *firstQ, double *median, double *thirdQ)
{
	double *errors = (double *) malloc(2 * cMerged->nbParticles * sizeof(double));
	*maximum = 0;
	*minimum = DBL_MAX;
	Cell *c;
	int s = 0;

	for (int cNo = 0; cNo < nbCells; cNo++)
	{
		c = cells + cNo;
		for (int pNo = 0; pNo < c->nbParticles; pNo++)
		{
			errors[2 * (s+pNo)] = relativeErr(c->fx[pNo], cMerged->fx[s + pNo]);
			errors[2 * (s+pNo) + 1] = relativeErr(c->fy[pNo], cMerged->fy[s + pNo]);
			*maximum = max(*maximum, max(errors[2 * (s+pNo)], errors[2 * (s+pNo) + 1]));
			*minimum = min(*minimum, min(errors[2 * (s+pNo)], errors[2 * (s+pNo) + 1]));
		}
		s += c->nbParticles;
	}

	qsort(errors, 2 * cMerged->nbParticles, sizeof(double), cmpDbl);

	*firstQ = errors[cMerged->nbParticles / 2];
	*median = errors[cMerged->nbParticles];
	*thirdQ = errors[3 * cMerged->nbParticles / 2];

	free(errors);
}

// Check that the cell c has the specified bounding box (with eps precision)
void checkCell(Cell *c, double xMin, double xMax, double yMin, double yMax, double eps)
{
	assert(fabs(c->xMin - xMin) < eps && 
		   fabs(c->xMax - xMax) < eps &&
		   fabs(c->yMin - yMin) < eps &&
		   fabs(c->yMax - yMax) < eps); 
}
