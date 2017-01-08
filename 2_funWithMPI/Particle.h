#ifndef PARTICLE_H
#define PARTICLE_H

#include "mpi.h"

#define G 6.67e-11 // m^3.kg^-1.s^-2

struct Particle_Shared
{
  double m; // mass
  double x; // x coordinate in the plane
  double y; // y coordinate in the plane
} typedef Particle_Shared;

struct Particle_Private;

struct Particle_Private
{
  double m; // mass

  double x; // x coordinate in the plane
  double y; // y coordinate in the plane

  double vx; // speed along x-axis
  double vy; // speed along y-axis

  double fx; // gravitationnal force exerted on the particle
  double fy; // gravitationnal force exerted on the particle

  double dmin; // distance between this particle and the closest particle.
} typedef Particle_Private;

MPI_Datatype MPI_PARTICLE;

/**
 * Registers the MPI type MPI_PARTICLE, corresponding
 * to the structure Particle_Shared
 */
void register_MPI_PARTICLE();

/**
 * Computes the interaction between the two specified particles,
 * and add it to the total forces exerted on them
 */
void addInteractions(Particle_Private *p1, Particle_Private *p2);

/**
 * Computes the force exerted by the first specified particle on the second
 * one, and add it to the total force exerted on the latter
 */
void addAction(Particle_Shared *p1, Particle_Private *p2);


/**
  * Returns a t such as:
  * sqrt[(vx*dt + ax*dt²/2)² + (vy*dt + ay*dt²/2)²] < r*dmin
  * For this, uses the triangular inequality by returning the exact highest dt such as:
  * sqrt(vx² + vy²) * dt + sqrt(ax² + ay²)/2 * dt² = r*dmin
  */
double dtUpperBound(Particle_Private *p, double r);

/**
 * Applies the applied force stored in the particle for the specified
 * time step
 */
 void applyForces(Particle_Private *p, double dt);

#endif
