#ifndef UTIL_H
#define UTIL_H

#include "Particle.h"
#include <stdio.h>

/**
 * Reads the nb of particles on the first line of the specified file
 * and returns it.
 * After this operation, inputFile points on the second line of the file.
 */
long parseNbParticles(FILE *inputFile);

/**
 * Parses the specified lines of the specified input file (counting from 1)
 * And returns a newly allocated array of particles in
 * the conditions described by the lines.
 */
Particle_Private *parseInitialConditions(FILE *inputFile, int firstLine, int nbLines);

/**
 * Write the coordinates
 * m x y vx vy\n
 * to the specified files for each particule of the specified array, in order
 */
void dumpSnapshot(FILE *outputFile, int nbParticles, Particle_Private *p);

/**
 * Creates the snapshot file of the specified iteration of the simulation
 * in the specified output folder
 * and initializes it by writing the number of particles on the first line.
 */
FILE *initSnapshotFile(char *outputDir, long snapshotNo, long nbParticles);

/**
 * Open the snapshot file of the specified iteration of the simulation
 * in the specified output folder in appending mode
 */
FILE *getSnapshotFile(char *outputDir, long snapshotNo);

#endif
