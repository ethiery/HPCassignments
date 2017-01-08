#include "Cell.h"
#include "Quadtree.h"
#include "utils.h"

#include <assert.h>
#include <math.h>
#include <stdlib.h>

#define EPS 1e-15
#define FAR_FIELD_LIMIT 0.707

// UNIT TESTS

void testP2M()
{
	printf("Testing center of mass computation: ");
	Cell c;
	initCell(&c, 2, 2, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0);
	c.nbParticles = 2;
	c.m[0] = 2.0;
	c.m[1] = 2.5;
	c.x[0] = 3.0;
	c.x[1] = 3.5;
	c.y[0] = 4.0;
	c.y[1] = 4.5;
	
	Multipole m;
	P2M(&m, &c);
	assert(fabs(m.m - 4.5) < EPS && fabs(m.x - 14.75/4.5) < EPS && fabs(m.y - 19.25/4.5) < EPS);

	freeCell(&c);

	printf("OK\n\n");
}

void testInitQuadtree()
{
	printf("Testing quadtree initialization (w/ Morton index): ");
	
	srand(42);
	Quadtree qt;
	initQuadtree(&qt, 3, 10, 20, 1.0, 2.0, 0.0, 1.0, 0.0, 1.0);

	checkCell(qt.cells + 0, 0.0, 0.25, 0.0, 0.25, EPS);
	checkCell(qt.cells + 1, 0.25, 0.5, 0.0, 0.25, EPS);
	checkCell(qt.cells + 2, 0.0, 0.25, 0.25, 0.5, EPS);
	checkCell(qt.cells + 3, 0.25, 0.5, 0.25, 0.5, EPS);
	checkCell(qt.cells + 4, 0.5, 0.75, 0.0, 0.25, EPS);
	checkCell(qt.cells + 5, 0.75, 1.0, 0.0, 0.25, EPS);
	checkCell(qt.cells + 6, 0.5, 0.75, 0.25, 0.5, EPS);
	checkCell(qt.cells + 7, 0.75, 1.0, 0.25, 0.5, EPS);
	checkCell(qt.cells + 8, 0.0, 0.25, 0.5, 0.75, EPS);
	checkCell(qt.cells + 9, 0.25, 0.5, 0.5, 0.75, EPS);
	checkCell(qt.cells + 10, 0.0, 0.25, 0.75, 1.0, EPS);
	checkCell(qt.cells + 11, 0.25, 0.5, 0.75, 1.0, EPS);
	checkCell(qt.cells + 12, 0.5, 0.75, 0.5, 0.75, EPS);
	checkCell(qt.cells + 13, 0.75, 1.0, 0.5, 0.75, EPS);
	checkCell(qt.cells + 14, 0.5, 0.75, 0.75, 1.0, EPS);
	checkCell(qt.cells + 15, 0.75, 1.0, 0.75, 1.0, EPS);

	freeQuadtree(&qt);

	printf("OK\n\n");
}

void testDistance()
{
	printf("Testing cell-particle distance computation: ");

	Cell c;
	Multipole m;
	srand(42);
	initCell(&c, 10000, 10000, 1e30, 1e32, 0, 1e17, 0, 1e17);

	initMultipole(&m, 1.0, -1e17, 2e17, 0.0, 0.0, 0.0, 0.0);
	assert(fabs(distance(&m, &c) - sqrt(1e17*1e17 + 1e17*1e17)) < EPS);
	initMultipole(&m, 1.0, 0.5e17, 2e17, 0.0, 0.0, 0.0, 0.0);
	assert(fabs(distance(&m, &c) - 1e17) < EPS);
	initMultipole(&m, 1.0, 2e17, 2e17, 0.0, 0.0, 0.0, 0.0);
	assert(fabs(distance(&m, &c) - sqrt(1e17*1e17 + 1e17*1e17)) < EPS);

	initMultipole(&m, 1.0, -1e17, 0.5e17, 0.0, 0.0, 0.0, 0.0);
	assert(fabs(distance(&m, &c) - 1e17) < EPS);
	initMultipole(&m, 1.0, 0.5e17, 0.5e17, 0.0, 0.0, 0.0, 0.0);
	assert(fabs(distance(&m, &c) - 0) < EPS);
	initMultipole(&m, 1.0, 2e17, 0.5e17, 0.0, 0.0, 0.0, 0.0);
	assert(fabs(distance(&m, &c) - 1e17) < EPS);

	initMultipole(&m, 1.0, -1e17, -1e17, 0.0, 0.0, 0.0, 0.0);
	assert(fabs(distance(&m, &c) - sqrt(1e17*1e17 + 1e17*1e17)) < EPS);
	initMultipole(&m, 1.0, 0.5e17, -1e17, 0.0, 0.0, 0.0, 0.0);
	assert(fabs(distance(&m, &c) - 1e17) < EPS);
	initMultipole(&m, 1.0, 2e17, -1e17, 0.0, 0.0, 0.0, 0.0);
	assert(fabs(distance(&m, &c) - sqrt(1e17*1e17 + 1e17*1e17)) < EPS);
	
	printf("OK\n\n");
}


// REGRESSION TESTS

void testPonP()
{
	printf("# Regression test 1, pOnP vs pOnP_ref (10k particles):\n");

	Cell c1, c2;
	srand(42);
	initCell(&c1, 10000, 10000, 1e30, 1e32, 0, 1e17, 0, 1e17);
	srand(42);
	initCell(&c2, 10000, 10000, 1e30, 1e32, 0, 1e17, 0, 1e17);

	ponP_ref(1e31, 0.5e17, 0.5e17, c1.nbParticles, c1.m, c1.x, c1.y, c1.fx, c1.fy); 
	
	WorkingVecs wv;
	initWorkingVecs(&wv);
	ponP(1e31, 0.5e17, 0.5e17, c2.nbParticles, c2.m, c2.x, c2.y, c2.fx, c2.fy, &wv); 
	
	double minRE, maxRE, firstQRE, medianRE, thirdQRE;
	computeRelativeErrors(&c1, 1, &c2, &minRE, &maxRE, &firstQRE, &medianRE, &thirdQRE);

	printf("# Relative errors : min, 1st quartile, median, 3rd quartile, max\n");
	printf("%e %e %e %e %e\n\n", minRE, firstQRE, medianRE, thirdQRE, maxRE);

	freeCell(&c1);
	freeCell(&c2);
	freeWorkingVecs(&wv);
}

void testP2Pin()
{
	printf("Regression test 2, P2P_in vs P2P_inRef (10k particles):\n");

	Cell c1, c2;
	srand(42);
	initCell(&c1, 10000, 10000, 1e30, 1e32, 0, 1e17, 0, 1e17);
	srand(42);
	initCell(&c2, 10000, 10000, 1e30, 1e32, 0, 1e17, 0, 1e17);

	P2P_inRef(&c1);

	WorkingVecs wv;
	initWorkingVecs(&wv);
	P2P_in(&c2, &wv);
	
	double minRE, maxRE, firstQRE, medianRE, thirdQRE;
	computeRelativeErrors(&c1, 1, &c2, &minRE, &maxRE, &firstQRE, &medianRE, &thirdQRE);

	printf("# Relative errors : min, 1st quartile, median, 3rd quartile, max\n");
	printf("%e %e %e %e %e\n\n", minRE, firstQRE, medianRE, thirdQRE, maxRE);

	freeCell(&c1);
	freeCell(&c2);
	freeWorkingVecs(&wv);
}

void testP2Pext()
{
	printf("Regression test 3, P2P_ext vs P2P_extRef (10k particles):\n");

	Cell c1, c2, c3;
	srand(42);
	initCell(&c1, 10000, 10000, 1e30, 1e32, 0, 1e17, 0, 1e17);
	srand(42);
	initCell(&c2, 10000, 10000, 1e30, 1e32, 0, 1e17, 0, 1e17);
	initCell(&c3, 10000, 10000, 1e30, 1e32, 0, 1e17, 0, 1e17);

	P2P_extRef(&c3, &c1);

	WorkingVecs wv;
	initWorkingVecs(&wv);
	P2P_ext(&c3, &c2, &wv);
	
	double minRE, maxRE, firstQRE, medianRE, thirdQRE;
	computeRelativeErrors(&c1, 1, &c2, &minRE, &maxRE, &firstQRE, &medianRE, &thirdQRE);

	printf("# Relative errors : min, 1st quartile, median, 3rd quartile, max\n");
	printf("%e %e %e %e %e\n\n", minRE, firstQRE, medianRE, thirdQRE, maxRE);

	freeCell(&c1);
	freeCell(&c2);
	freeCell(&c3);
	freeWorkingVecs(&wv);
}

void testBarnesHut()
{
	srand(42);
	printf("Regression test 4, comparing results of a Barnes-Hut like method on a 4x4 grid" 
		   "(10 k particles) VS P2P_inRef on one global cell:\n");

	Cell cMerged;
	Quadtree qt;
	initQuadtree(&qt, 3, 10000, 10000, 1e30, 1e32, 0, 1e17, 0, 1e17);
	mergeCell(&cMerged, qt.nbCells, qt.cells);

	P2P_inRef(&cMerged);

	computeMultipoles(&qt);
	computeForces(&qt, FAR_FIELD_LIMIT);	

	double minRE, maxRE, firstQRE, medianRE, thirdQRE;
	computeRelativeErrors(&cMerged, qt.nbCells, qt.cells, &minRE, &maxRE, &firstQRE, &medianRE, &thirdQRE);

	printf("# Relative errors : min, 1st quartile, median, 3rd quartile, max\n");
	printf("%e %e %e %e %e\n\n", minRE, firstQRE, medianRE, thirdQRE, maxRE);

	freeCell(&cMerged);
	freeQuadtree(&qt);
}


int main(int argc, char const *argv[])
{
	testP2M();
	testInitQuadtree();
	testDistance();

	testPonP();
	testP2Pin();
	testP2Pext();
	testBarnesHut();

	return EXIT_SUCCESS;
}