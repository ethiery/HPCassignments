#ifndef UTIL_H
#define UTIL_H


void parseMatrixFile(char *inputFilePath, double **mat, int *M, int *N);

void writeMatrixFile(int M, int N, double *mat, char *outputFilePath);

#endif
