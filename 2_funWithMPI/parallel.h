#ifndef PARALLEL_H
#define PARALLEL_H

#include "Particle.h"

/**
 * Assumes that inputFileName is a path to a valid input file
 */
void parallelSimulation(char* inputFileName, int nbSteps, double r,
                   char* outputDir, int outputPeriod);

#endif
