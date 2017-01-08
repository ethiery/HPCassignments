#ifndef MULTIPOLE_H
#define MULTIPOLE_H

#include <malloc.h>

// Multiple
struct Multipole
{
  double m; // mass
  double x; // x coordinate in the plane
  double y; // y coordinate in the plane

  // Boundaries of the cell approximated by the multipole
  double xMin;
  double xMax;
  double yMin;
  double yMax;
} 
typedef Multipole;

// Initialize a multipole
extern void initMultipole(Multipole *mp, double m, double x, double y, 
						  double xMin, double xMax, double yMin, double yMax);


#endif
