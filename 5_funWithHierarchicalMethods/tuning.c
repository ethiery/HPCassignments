#include <stdlib.h>

#include "Cell.h"
#include "Multipole.h"
#include "utils.h"
#include "WorkingVecs.h"

// private

static void clearForces(Cell *c)
{
	for (int i = 0; i < c->nbParticles; i++)
	{
		c->fx[i] = 0;
		c->fy[i] = 0;
	}
}


// TUNING TEST

void tuneM2P()
{
	printf("# Tuning the MAC. Force exerted by a cell on another cell of 10k particles. "
		   "M2P vs P2P_extRef.\n\n");


	// Setup cells c0 and c1, two identical cells
	Cell c0, c1, c2;
	Multipole m2;
	WorkingVecs wv;
	srand(42);
	initCell(&c0, 10000, 10000, 1.0, 100.0, 0, 1e17, 0, 1e17);
	srand(42);
	initCell(&c1, 10000, 10000, 1.0, 100.0, 0, 1e17, 0, 1e17);
	initWorkingVecs(&wv);

	double minRE, maxRE, firstQRE, medianRE, thirdQRE, d;
	double l = 1e17;

	printf("# Influence of the MAC, 10k particles per cell:\n"
		   "# l/d Relative errors(min, 1st quartile, median, 3rd quartile, max)\n");
	
	for (double i = 1e17; i < 4e17; i += 0.25e17)
	{
		initCell(&c2, 10000, 10000, 1, 100.0, i, i+1e17, i, i+1e17);
		P2M(&m2, &c2);

		P2P_extRef(&c2, &c0);
		M2P(&m2, &c1, &wv);

		d = distance(&m2, &c0);		
		computeRelativeErrors(&c0, 1, &c1, &minRE, &maxRE, &firstQRE, &medianRE, &thirdQRE);
		printf("%e %e %e %e %e %e\n", l/d, minRE, firstQRE, medianRE, thirdQRE, maxRE);

		clearForces(&c0);
		clearForces(&c1);
		freeCell(&c2);
	}

	freeWorkingVecs(&wv);
}

int main(int argc, char const *argv[])
{
	tuneM2P();

	return EXIT_SUCCESS;
}