#include <assert.h>
#include <float.h>
#include <stdio.h>
#include <stdlib.h>


#include "util.h"

/**
 * Reads the nb of particles on the first line of the specified file
 * and returns it.
 * After this operation, inputFile points on the second line of the file.
 */
long parseNbParticles(FILE *inputFile)
{
  assert(fseek(inputFile, 0, SEEK_SET) == 0);
  long n;
  assert(fscanf(inputFile, "%ld\n", &n) == 1);
  return n;
}

/**
 * Parses the specified lines of the specified input file
 *  (line containing the number of lines is line -1)
 * And returns a newly allocated array of particles in
 * the conditions described by the lines.
 */
Particle_Private *parseInitialConditions(FILE *inputFile, int firstLine, int nbLines)
{
  assert(inputFile != NULL);

  long n = parseNbParticles(inputFile);
  assert(0 <= firstLine && firstLine < n);
  assert(0 < nbLines);
  assert(firstLine + nbLines <= n);

  Particle_Private *p = (Particle_Private *) malloc(nbLines * sizeof(Particle_Private));
  // Skips previous lines
  int i;
  for (i = 0; i < firstLine; i++)
  {
    assert(fscanf(inputFile, "%*[^\n]\n") == 0);
  }
  // Read specified lines
  for (i = 0; i < nbLines; i++)
  {
    assert(fscanf(inputFile, "%lf %lf %lf %lf %lf\n",
      &p[i].m, &p[i].x, &p[i].y, &p[i].vx, &p[i].vy) == 5);
    p[i].fx = p[i].fy = 0;
    p[i].dmin = DBL_MAX;
  }

  return p;
}

/**
 * Creates the snapshot file of the specified iteration of the simulation
 * in the specified output folder
 * and initializes it by writing the number of particles on the first line.
 */
FILE *initSnapshotFile(char *outputDir, long snapshotNo, long nbParticles)
{
  char outputFileName[1000];
  sprintf(outputFileName, "%s/%ld.dat", outputDir, snapshotNo);
  FILE *f = fopen(outputFileName, "w");
  assert(f != NULL);
  fprintf(f, "%ld\n", nbParticles);
  return f;
}

/**
 * Open the snapshot file of the specified iteration of the simulation
 * in the specified output folder in appending mode
 */
FILE *getSnapshotFile(char *outputDir, long snapshotNo)
{
  char outputFileName[1000];
  sprintf(outputFileName, "%s/%ld.dat", outputDir, snapshotNo);
  FILE *f = fopen(outputFileName, "a");
  assert(f != NULL);
  return f;
}

/**
 * Write the coordinates
 * m x y vx vy\n
 * to the specified files for each particule of the specified array, in order
 */
void dumpSnapshot(FILE *outputFile, int nbParticles, Particle_Private *p)
{
  assert(outputFile != NULL);
  for (int i = 0; i < nbParticles; i++)
  {
    assert(fprintf(outputFile, "%.5e %.5e %.5e %.5e %.5e\n",
      p[i].m, p[i].x, p[i].y, p[i].vx, p[i].vy) > 0);
  }
}
