#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include "parallel.h"
#include "sequential.h"
#include "util.h"

void printUsage() {
  printf("Usage: ./simulator -i pathToInitialState [-s] [-o outputDir] [-p 1] [-n 100] [-r 0.1]\n"
         "-s : uses sequential version\n"
         "-o : does not measure the duration of the simulation, and outputs the state"
         "every P states, in a file of the specified directory\n"
         "-p : specifies the period of generation of the output files"
         "-n : defines the number of iterations, default is 10\n"
         "-i : specifies a path to the file containing the initial state of the particles\n"
         "-r : specifies the ratio of the distance to its closest particle a particle is allowed to travel in one step\n"
       );
}

typedef void (*t_simulation) (char*, int, double, char*, int);

int main(int argc, char**argv)
{
  // Parses command line arguments
  int opt;
  t_simulation simFunc = parallelSimulation;
  char inputFile[1000];
  char outputDir[1000];
  int outputEnabled = 0;
  int outputPeriod = 1;
  int nbIterations = 1000;
  double r = 0.1;

  while ((opt = getopt(argc, argv, "si:o:p:n:r:")) != -1) {
    switch (opt) {
      // Sequential vs MPI version, default is MPI
      case 's':
        simFunc = seqSimulation;
        break;
      // Input file
      case 'i':
        strncpy(inputFile, optarg, 1000);
        break;
      // Output dir
      case 'o':
        strncpy(outputDir, optarg, 1000);
        outputEnabled = 1;
        break;
      // Output period
      case 'p':
        outputPeriod = atoi(optarg);
        break;
      // Number of iterations
      case 'n':
        nbIterations = atoi(optarg);
        break;
      case 'r':
        r = atof(optarg);
        break;
      default:
        printUsage();
        abort();
    }
  }

  // Check arguments
  FILE *checkFile = fopen(inputFile, "r");
  if (checkFile == NULL)
  {
    printf("Invalid or missing input file\n");
    printUsage();
    abort();
  }
  fclose(checkFile);


  if (outputEnabled)
  {
    struct stat st = {0};
    if (stat(outputDir, &st) != -1)
    {
      // printf("Warning: specified output directory already exists. Overwritting.\n");
      // outputEnabled = 0;
    } else {
      mkdir(outputDir, 0775);
    }
  }

  struct timeval start, stop;
  gettimeofday(&start, NULL);
  simFunc(inputFile, nbIterations, r, outputEnabled ? outputDir : NULL, outputPeriod);
  gettimeofday(&stop, NULL);

  printf("%lf\n", stop.tv_sec - start.tv_sec + 0.000001 * (stop.tv_usec - start.tv_usec));
}
