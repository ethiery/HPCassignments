#include "Multipole.h"

#include <math.h>

// Initialize a multipole
void initMultipole(Multipole *mp, double m, double x, double y, 
				   double xMin, double xMax, double yMin, double yMax)

{
  mp->m = m;
  mp->x = x;
  mp->y = y;
  mp->xMin = xMin;
  mp->xMax = xMax;
  mp->yMin = yMin;
  mp->yMax = yMax;
}