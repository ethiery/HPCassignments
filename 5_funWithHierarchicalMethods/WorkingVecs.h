#ifndef WORKING_VECS_H
#define WORKING_VECS_H

// Working buffers initialized beforehand to prevent multiple initialization.
// Each buffer is of size "size"
// unitVec is filled with ones.
typedef struct WorkingVecs
{
	int size;
	double *unitVec;
	double *v1;
	double *v2;
	double *v3;
	double *v4;
	double *v5;
} WorkingVecs;

// Initialize a set of working buffers
// Note: one different set should be used by each process 
void initWorkingVecs(WorkingVecs *wv);

// Makes sure the size of working buffers is at least newSize 
void resizeWorkingVecs(WorkingVecs *wv, int newSize);

// Release resources associated with a set of working buffers
void freeWorkingVecs(WorkingVecs *wv);

#endif