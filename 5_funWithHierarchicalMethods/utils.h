#ifndef UTILS_H
#define UTILS_H

#include "Cell.h"

// HELPER METHODS

extern double min(double a, double b);

extern double max(double a, double b);

extern double dist(double x1, double y1, double x2, double y2);

extern double randDouble(double minVal, double maxVal);


// Compute the relative errors between forces applied on the particles of cell "cMerged"
// And particles in the array of cell "cells" (in order).
// Sets the min, max, 1st, 2nd and 3rd quartiles
// relative erros are d(fx)/fx, and d(fy)/fy
extern void computeRelativeErrors(Cell *cMerged, int nbCells, Cell *cells, 
								  double *minimum, double *maximum, double *firstQ, double *median, double *thirdQ);

// Check that the cell c has the specified bounding box (with eps precision)
extern void checkCell(Cell *c, double xMin, double xMax, double yMin, double yMax, double eps);


#endif