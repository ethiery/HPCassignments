#include "Cell.h"
#include "utils.h"

#include <float.h>
#include <malloc.h>
#include <math.h>
#include <mkl.h>
#include <stdlib.h>
#include <string.h>


#define G 6.67e-11 // m^3.kg^-1.s^-2


// Initialize a cell with a random number of particles between
// nbParticlesMin and nbParticlesMax, of random mass between mMin and mMax, and random 
// positions (x,y) with x between xMin and xMax and y between yMin and yMax
void initCell(Cell *c, int nbParticlesMin, int nbParticlesMax, double mMin, double mMax, double xMin, double xMax, double yMin, double yMax)
{
	c->xMin = xMin; c->xMax = xMax; c->yMin = yMin; c->yMax = yMax;
	c->nbParticles = randDouble(nbParticlesMin, nbParticlesMax);
	c->m = (double *) malloc(5 * c->nbParticles * sizeof(double));
	c->x = c->m + c->nbParticles;
	c->y = c->x + c->nbParticles;
	c->fx = c->y + c->nbParticles;
	c->fy = c->fx + c->nbParticles;

	for (int i = 0; i < c->nbParticles; i++)
	{
		c->m[i] = randDouble(mMin, mMax);
		c->x[i] = randDouble(xMin, xMax);
		c->y[i] = randDouble(yMin, yMax);
		c->fx[i] = 0;
		c->fy[i] = 0;
	}
}

// Merge cells in cMerged
void mergeCell(Cell *cMerged, int nbCells, Cell *cells)
{
	cMerged->nbParticles = 0;
	cMerged->xMin = cMerged->yMin = DBL_MAX;
	cMerged->xMax =  cMerged->yMax = DBL_MIN;

	for (int i = 0; i < nbCells; i++)
	{
		cMerged->nbParticles += cells[i].nbParticles;
		cMerged->xMin = min(cells[i].xMin, cMerged->xMin); 
		cMerged->xMax = max(cells[i].xMax, cMerged->xMax); 
		cMerged->yMin = min(cells[i].yMin, cMerged->yMin); 
		cMerged->yMax = max(cells[i].yMax, cMerged->yMax); 
	}

	cMerged->m = (double *) malloc(5 * cMerged->nbParticles * sizeof(double));
	cMerged->x = cMerged->m + cMerged->nbParticles;
	cMerged->y = cMerged->x + cMerged->nbParticles;
	cMerged->fx = cMerged->y + cMerged->nbParticles;
	cMerged->fy = cMerged->fx + cMerged->nbParticles;

	int p = 0;
	for (int i = 0; i < nbCells; i++)
	{
		memcpy(cMerged->m + p, cells[i].m, cells[i].nbParticles * sizeof(double));
		memcpy(cMerged->x + p, cells[i].x, cells[i].nbParticles * sizeof(double));
		memcpy(cMerged->y + p, cells[i].y, cells[i].nbParticles * sizeof(double));
		memcpy(cMerged->fx + p, cells[i].fx, cells[i].nbParticles * sizeof(double));
		memcpy(cMerged->fy + p, cells[i].fy, cells[i].nbParticles * sizeof(double));
		p += cells[i].nbParticles;
	}
}

// Release ressources associated with the cell c
void freeCell(Cell *c)
{
	free(c->m);
}

// Returns the distance between the bounding box of a multipole and a cell
double distance(Multipole *m, Cell *c)
{
  double d = 0;

  if (m->x < c->xMin)
  {
    if (m->y < c->yMin)
      d = dist(c->xMin, c->yMin, m->x, m->y);
    else if (m->y > c->yMax)
      d = dist(c->xMin, c->yMax, m->x, m->y);
    else
      d = c->xMin - m->x;
  }
  else if (m->x > c->xMax)
  {
    if (m->y < c->yMin)
      d = dist(c->xMax, c->yMin, m->x, m->y);
    else if (m->y > c->yMax)
      d = dist(c->xMax, c->yMax, m->x, m->y);
    else
      d = m->x - c->xMax;
  }
  else
  {
    if (m->y < c->yMin)
      d = c->yMin - m->y;
    else if (m->y > c->yMax)
      d = m->y - c->yMax;
  }

  return d;
}




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
void ponP(double mC, double xC, double yC,
		 double nbParticles, double *m, double *x, double *y, double *fx, double *fy,
		 WorkingVecs *wv)
{
	if (nbParticles > 0)
	{
		if (nbParticles > wv->size)
			resizeWorkingVecs(wv, nbParticles);

		// v1 = xI - xC
		// v2 = (xI - xC)²
		memcpy(wv->v1, x, nbParticles * sizeof(double));
		cblas_daxpy(nbParticles, -xC, wv->unitVec, 1, wv->v1, 1);
		vdMul(nbParticles, wv->v1, wv->v1, wv->v2);
		
		// v3 = yI - yC
		// v4 = (yI - yC)²
		memcpy(wv->v3, y, nbParticles  * sizeof(double));
		cblas_daxpy(nbParticles, -yC, wv->unitVec, 1, wv->v3, 1);
		vdMul(nbParticles, wv->v3, wv->v3, wv->v4);

		// v5 = dI = (xI - xC)² + (yI - yC)²
		// v4 = dI^(3/2)
		// v2 = mI / dI^(3/2)
		vdAdd(nbParticles, wv->v2, wv->v4, wv->v5);
		vdPow3o2(nbParticles, wv->v5, wv->v4);
		vdDiv(nbParticles, m, wv->v4, wv->v2);

		// v4 = (xI-xC) * mi / dI^(3/2)
		// v5 = (yI-yC) * mi / dI^(3/2)
		vdMul(nbParticles, wv->v1, wv->v2, wv->v4);
		vdMul(nbParticles, wv->v3, wv->v2, wv->v5);

		// fx = fx - G * mC * (xI-xC) * mi / dI^(3/2)
		// fy = fy - G * mC * (yI-yC) * mi / dI^(3/2)
		cblas_daxpy(nbParticles, -G * mC, wv->v4, 1, fx, 1);
		cblas_daxpy(nbParticles, -G * mC, wv->v5, 1, fy, 1);
	}
}

// Same as pOnP, but naive version for regression tests 
void ponP_ref(double mC, double xC, double yC, double nbParticles, double *m, double *x, 
		  double *y, double *fx, double *fy)
{
	for (int i = 0; i < nbParticles; i++)
	{
		double dx = xC - x[i];
		double dy = yC - y[i];
		double r = G * mC * m[i] / pow(dx*dx + dy*dy, 1.5);
		fx[i] += r * dx;
		fy[i] += r * dy;
	}
}


// Apply the forces of the particles in c on each other
void P2P_in(Cell *c, WorkingVecs *wv)
{
	for (int i = 0; i < c->nbParticles; i++)
	{
		ponP(c->m[i], c->x[i], c->y[i], i, c->m, c->x, c->y, c->fx, c->fy, wv);
		ponP(c->m[i], c->x[i], c->y[i], c->nbParticles - (i+1), 
			c->m + (i+1), c->x + (i+1), c->y + (i+1), c->fx + (i+1), c->fy + (i+1),
			wv);
	}
}

// Apply the forces of the particles in c on each other naively
// (for regression test);
void P2P_inRef(Cell *c)
{
	for (int i = 0; i < c->nbParticles; i++)
	{
		ponP_ref(c->m[i], c->x[i], c->y[i], i, c->m, c->x, c->y, c->fx, c->fy);
		ponP_ref(c->m[i], c->x[i], c->y[i], c->nbParticles - (i+1), 
				 c->m + (i+1), c->x + (i+1), c->y + (i+1), c->fx + (i+1), c->fy + (i+1));
	}
}

// Apply the forces of the particles in c1 on the particles in c2
void P2P_ext(Cell *c1, Cell *c2, WorkingVecs *wv)
{
	for (int i = 0; i < c1->nbParticles; i++)
		ponP(c1->m[i], c1->x[i], c1->y[i], c2->nbParticles, c2->m, c2->x, c2->y, c2->fx, c2->fy, wv);	
}

// Apply the forces of the particles in c1 on the particles in c2
// naively (for regression test)
void P2P_extRef(Cell *c1, Cell *c2)
{
	for (int i = 0; i < c1->nbParticles; i++)
		ponP_ref(c1->m[i], c1->x[i], c1->y[i], c2->nbParticles, c2->m, c2->x, c2->y, c2->fx, c2->fy);	
}


// Approximate cell c by a multipole m
void P2M(Multipole *m, Cell *c)
{
	m->m = m->x = m->y = 0;
	m->xMin = c->xMin;
	m->xMax = c->xMax;
	m->yMin = c->yMin;
	m->yMax = c->yMax;

	for (int i = 0; i < c->nbParticles; i++)
	{
		m->m += c->m[i];
		m->x += c->m[i] * c->x[i];
		m->y += c->m[i] * c->y[i];
	}

	m->x /= m->m;
	m->y /= m->m;
}

// Apply the forces of a multipole m on particles in cell c
void M2P(Multipole *m, Cell *c, WorkingVecs *wv)
{
	ponP(m->m, m->x, m->y, c->nbParticles, c->m, c->x, c->y, c->fx, c->fy, wv);
}

// Approximate sub-multipoles sm by a multipole m
void M2M(Multipole *m, int nbSms, Multipole *sms)
{
	m->m = m->x = m->y = 0;
	m->xMin = m->yMin = DBL_MAX;
	m->xMax = m->yMax = DBL_MIN;

	for (int i = 0; i < nbSms; i++)
	{
		m->m += sms[i].m;
		m->x += sms[i].m * sms[i].x;
		m->y += sms[i].m * sms[i].y;
		m->xMin = min(m->xMin, sms[i].xMin);
		m->xMax = max(m->xMax, sms[i].xMax);
		m->yMin = min(m->yMin, sms[i].yMin);
		m->yMax = max(m->yMax, sms[i].yMax);
	}

	m->x /= m->m;
	m->y /= m->m;	
}


