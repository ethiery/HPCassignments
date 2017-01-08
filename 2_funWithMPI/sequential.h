#ifndef SEQUENTIAL_H
#define SEQUENTIAL_H

#include "Particle.h"

/**
 * Assumes that inputFileName is a path to a valid input file
 */
void seqSimulation(char* inputFileName, int nbSteps, double r,
                   char* outputDir, int outputPeriod);

#endif
