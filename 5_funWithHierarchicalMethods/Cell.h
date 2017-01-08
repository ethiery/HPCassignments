#ifndef FAST_CELL_H
#define FAST_CELL_H

#include "Multipole.h"
#include "WorkingVecs.h"

typedef struct Cell
{
	// Boundaries of the cell
	double xMin;
	double xMax;
	double yMin;
	double yMax;

	// Particles
	int nbParticles;
	double *m;
	double *x;
	double *y;
	double *fx;
	double *fy;
} Cell;


// Initialize a cell with a random number of particles between
// nbParticlesMin and nbParticlesMax, of random mass between mMin and mMax, and random 
// positions (x,y) with x between xMin and xMax and y between yMin and yMax
extern void initCell(Cell *c, int nbParticlesMin, int nbParticlesMax, double mMin, double mMax, 
					 double xMin, double xMax, double yMin, double yMax);

// Merge cells in cMerged
extern void mergeCell(Cell *cMerged, int nbCells, Cell *cells);

// Release ressources associated with the cell c
extern void freeCell(Cell *c);

// Returns the distance between the bounding box of a multipole and a cell
extern double distance(Multipole *m, Cell *c);


// Compute the respective forces (fx[i], fy[i]) exerted by a particle of mass mC in (xC, yC) 
// over nbParticles of respective masses m[i] in (x[i], y[i]) 
//
// Note: the module of the gravitational force exerted by a particle PC (mC, xC, yC) 
// on a particle P_I (mI, xI, yI) is:
// 
// 		F = G * mI * mC / dI      where dI = (xI - xC)² + (yI - yC)² 
// 
// i.e projected on x axis : - ((xI-xC) / sqrt(dI)) * F = - G * mC * (xI-xC) * mi / dI^(3/2)
// and projected on y axis : - ((yI-yC) / sqrt(dI)) * F = - G * mC * (yI-yC) * mi / dI^(3/2)
// 
extern void ponP(double mC, double xC, double yC, double nbParticles, double *m, double *x, 
				 double *y, double *fx, double *fy, WorkingVecs *wv);

// Same as pOnP, but naive version for regression tests 
extern void ponP_ref(double mC, double xC, double yC, double nbParticles, double *m, 
					 double *x, double *y, double *fx, double *fy);

// Apply the forces of the particles in c on each other
extern void P2P_in(Cell *c, WorkingVecs *wv);

// Same as P2P_in, but naive version for regression tests
extern void P2P_inRef(Cell *c);

// Apply the forces of the particles in c1 on the particles in c2
extern void P2P_ext(Cell *c1, Cell *c2, WorkingVecs *wv);

// Apply the forces of the particles in c1 on the particles in c2
// naively (for regression test)
extern void P2P_extRef(Cell *c1, Cell *c2);

// Approximate cell c by a multipole m
extern void P2M(Multipole *m, Cell *c);

// Apply the forces of a multipole m on particles in cell c
extern void M2P(Multipole *m, Cell *c, WorkingVecs *wv);

// Approximate sub-multipoles sm by a multipole m
extern void M2M(Multipole *m, int nbSms, Multipole *sms);


#endif