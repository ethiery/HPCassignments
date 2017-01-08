#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main (int argc, char **argv)
{
	if (argc != 4)
	{
		fprintf(stderr, "Usage: %s nbLines nbColumns outputFileName\n", argv[0]);
		return EXIT_FAILURE;
	}

	FILE *f = fopen(argv[3], "w");
	if (f == NULL)
	{
		fprintf(stderr, "Error opening output file\n");
		return EXIT_FAILURE;
	}

	int n = atoi(argv[1]);
	int m = atoi(argv[2]);
	srand((unsigned int)time(NULL));
	float min = -100;
	float max = 100;
	float randomFloat;

	fprintf(f, "%d %d\n", n, m);

	for (int row = 0; row < n; row++)
	{
		for (int col = 0; col < m; col++)
		{
			randomFloat = ((float)rand()/(float)(RAND_MAX)) * (max - min) + min;
			fprintf(f, "%.3f ", randomFloat);
		}
		fprintf(f, "\n");
	}
	fclose(f);
}