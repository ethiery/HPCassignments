#include "Particle.h"
#include "mpi.h"

#include <assert.h>
#include <float.h>
#include <math.h>
#include <stdio.h>

/**
 * Registers the MPI type MPI_PARTICLE, corresponding
 * to the structure Particle_Shared
 */
void register_MPI_PARTICLE()
{
  int count = 3;
  int blockLengths[3] = { 1, 1, 1 };
  MPI_Datatype types[3] = { MPI_DOUBLE, MPI_DOUBLE, MPI_DOUBLE };
  MPI_Aint displacements[3];

  MPI_Aint i1, i2;

  Particle_Shared p;
  MPI_Get_address(&p, &i1);
  MPI_Get_address(&p.m, &i2);
  displacements[0] = i2 - i1;
  MPI_Get_address(&p.x, &i2);
  displacements[1] = i2 - i1;
  MPI_Get_address(&p.y, &i2);
  displacements[2] = i2 - i1;


  MPI_Type_create_struct(count, blockLengths, displacements, types, &MPI_PARTICLE);
  MPI_Type_commit(&MPI_PARTICLE);
}

/**
 * Computes the interaction between the two specified particles,
 * and adds it to the force exterted on them
 */
void addInteractions(Particle_Private *p1, Particle_Private *p2)
{
  // Computes the force exerted by p1 on p2 and vice versa
  double dx = p2->x - p1->x;
  double dy = p2->y - p1->y;
  assert(dx != 0 || dy != 0);

  double dist = sqrt(dx*dx + dy*dy);
  double r = G*p1->m*p2->m / (dist*dist*dist);

  double fx = r * dx;
  double fy = r * dy;
  p1->fx += fx;
  p1->fy += fy;
  p2->fx -= fx;
  p2->fy -= fy;
  // Updates the distance with the closest particle of both p1 and p2
  p1->dmin = (dist < p1->dmin) ? dist : p1->dmin;
  p2->dmin = (dist < p2->dmin) ? dist : p2->dmin;
}

/**
 * Computes the force exerted by the first specified particle on the second
 * one, and add it to the total force exerted on the latter
 */
void addAction(Particle_Shared *p1, Particle_Private *p2)
{
  // Computes the force exerted by p1 on p2 and vice versa
  double dx = p2->x - p1->x;
  double dy = p2->y - p1->y;
  if (dx == 0 && dy == 0) return;

  double dist = sqrt(dx*dx + dy*dy);
  double r = G*p1->m*p2->m / (dist*dist*dist);
  p2->fx -= r * dx;;
  p2->fy -= r * dy;
  // Updates the distance with the closest particle of both p1 and p2
  p2->dmin = (dist < p2->dmin) ? dist : p2->dmin;
}



/**
  * Computes and returns a time step dt such as the distance travelled
  * by the specified particle during this dt is lower than a specified ratio
  * of the distance between the particle and its closest neighbour.
  *
  * Technically, computes and returns the exact highest dt such as:
  * sqrt(vx² + vy²) * dt + sqrt(ax² + ay²)/2 * dt² = r*dmin
  */
double dtUpperBound(Particle_Private *p, double r)
{
  assert(0 < r && r < 1);
  double v = sqrt(p->vx*p->vx + p->vy*p->vy);
  double a = sqrt(p->fx*p->fx + p->fy*p->fy) / p->m;
  double delta = v*v + 2*a*r*p->dmin;
  return (sqrt(delta) - v) / a;
}

/**
 * Applies the applied force stored in the particle for the specified
 * time step
 */
 void applyForces(Particle_Private *p, double dt)
 {
   double ax = p->fx/p->m;
   double ay = p->fy/p->m;
   p->x += p->vx*dt + ax*dt*dt/2;
   p->y += p->vy*dt + ay*dt*dt/2;
   p->vx += ax*dt;
   p->vy += ay*dt;
   p->fx = p->fy = 0;
   p->dmin = DBL_MAX;
 }
