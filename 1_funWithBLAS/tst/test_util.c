#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "benchtools.h"
#include "util.h"


int test_ddisp(int verbose)
{
  int ret;
  char streamName[] = "tmp";
  char expected[] = "2.000e+00 7.000e+00 1.200e+01 \n3.000e+00 8.000e+00 1.300e+01 \n";
  FILE *stream = fopen(streamName, "w+");
  double *a = dmat(5, 5);
  for (int i = 0; i < 25; i++) {
    a[i] = i;
  }

  ddisp(2, 3, a+2, 5, stream);

  // Checks ddisp wrote a valid number of characters to stream
  long size = ftell(stream);
  if (size != (int) strlen(expected))
    FAIL("test_ddisp: ddisp wrote an unexpected number of chars to stream\n");

  // Reads what ddisp wrote to stream
  ret = fseek(stream, 0, SEEK_SET);
  if (ret != 0)
    FAIL("test_ddisp: error while going back to beginning of the stream\n");

  char result[size];
  ret = fread(result, sizeof(char), size, stream);
  if (ret != size)
    FAIL("test_ddisp: error while reading result of ddisp\n");

  // Compares is with the expected string
  if(strcmp(result, expected) != 0)
    FAIL("test_ddisp: ddisp wrote unexpected content to stream\n");

  fclose(stream);
  unlink(streamName);

  SUCCESS("test_ddisp: passed\n");
}
