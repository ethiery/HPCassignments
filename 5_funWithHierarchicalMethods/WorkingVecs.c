#include "WorkingVecs.h"

#include <malloc.h>

#define INIT_SIZE 100
#define NB_VECS 6
#define ONE 1.0

// Initialize a set of working buffers
// Note: one different set should be used by each process 
void initWorkingVecs(WorkingVecs *wv)
{
	wv->size = INIT_SIZE;
	wv->unitVec = (double *) malloc(NB_VECS * wv->size * sizeof(double));
	for (int i = 0; i < wv->size; i++) 
		wv->unitVec[i] = ONE;

	wv->v1 = wv->unitVec + wv->size;
	wv->v2 = wv->v1 + wv->size;
	wv->v3 = wv->v2 + wv->size;
	wv->v4 = wv->v3 + wv->size;
	wv->v5 = wv->v4 + wv->size;
}

// Makes sure the size of working buffers is at least newSize 
void resizeWorkingVecs(WorkingVecs *wv, int newSize)
{
	wv->unitVec = (double *) realloc(wv->unitVec, NB_VECS * newSize * sizeof(double));
	for (int i = wv->size ; i < newSize; i++)
		wv->unitVec[i] = ONE;

	wv->size = newSize;
	wv->v1 = wv->unitVec + wv->size;
	wv->v2 = wv->v1 + wv->size;
	wv->v3 = wv->v2 + wv->size;
	wv->v4 = wv->v3 + wv->size;
	wv->v5 = wv->v4 + wv->size;
}

// Release resources associated with a set of working buffers
void freeWorkingVecs(WorkingVecs *wv)
{
	free(wv->unitVec);
}