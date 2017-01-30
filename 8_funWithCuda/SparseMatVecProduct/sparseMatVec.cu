#include <cuda_runtime.h>
#include "cublas_v2.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

//#define DISPLAY

typedef struct MatCSR
{
	int M;
	int N;
	int *ptrs; // row ptrs + nbNZ, size M+1
	int *idx; // col indices, size nbNZ
	float *vals; // values, size nbNZ
} MatCSR;

static void
initRandomMatCSR(MatCSR *mat, int M, int N, float density, float min, float max)
{
	int nbNZ = M*N*density;
	mat->M = M;
	mat->N = N;
	mat->ptrs = (int *) malloc((M+1) * sizeof(int));
	mat->idx = (int *) malloc(nbNZ * sizeof(int));
	mat->vals = (float *) malloc(nbNZ * sizeof(float));

	int currentPtr = 0;
	int currentIdx = 0;
	for (int row = 0; row < M; row++)
	{
		mat->ptrs[currentPtr++] = currentIdx;
		for (int col = 0; col < N; col++)
		{
			if ((currentIdx < nbNZ) && ((float)rand() / (float)RAND_MAX) < density)
			{
				mat->idx[currentIdx] = col;
				mat->vals[currentIdx++] = min + (max - min) * ((float)rand() / (float)RAND_MAX);
			}
		}
	}
	mat->ptrs[currentPtr] = currentIdx;
}

static void
freeMatCSR(MatCSR *mat)
{
	free(mat->ptrs);
	free(mat->idx);
	free(mat->vals);
}

static void 
sparseGemv(float *y, MatCSR *A, float *x)
{
	float dot;
	int rowStart, rowEnd;

	for (int row = 0; row < A->M; row++)
	{
		dot = 0;
		rowStart = A->ptrs[row];
		rowEnd = A->ptrs[row+1]; 
		for (int i = rowStart; i < rowEnd; i++)
			dot += A->vals[i] * x[A->idx[i]];
		y[row] = dot;
	}
}

__global__ void sparseGemvGPU(float *y, int *ptrs, int *idx, float *vals, float *x)
{
    int row = blockIdx.x * blockDim.x + threadIdx.x;
    float dot = 0;
    for (int i = ptrs[row]; i < ptrs[row+1]; i++)
        dot += vals[i] * x[idx[i]];
    y[row] = dot;
}

__global__ void sparseGemvGPU2(float *y, int *ptrs, int *idx, float *vals, float *x)
{
    int row = blockIdx.x * blockDim.x + threadIdx.x;
    int nbValues = ptrs[row+1] - ptrs[row];
    __shared__ float values[nbValues];
    for (int i = 0; i < nbValues; i++)
        values[i] = vals[ptrs[row] + i];

    float dot = 0;
    for (int i = 0; i < nbValues; i++)
        dot += values[i] * x[idx[ptrs[row] + i]];
    y[row] = dot;
}

static void
sparseToDense(float *denseMat, MatCSR *sparseMat)
{
	memset(denseMat, 0, sparseMat->M * sparseMat->N * sizeof(float));

	int rowStart, rowEnd;
	for (int row = 0; row < sparseMat->M; row++)
	{
		rowStart = sparseMat->ptrs[row];
		rowEnd = sparseMat->ptrs[row+1]; 
		for (int i = rowStart; i < rowEnd; i++)
			denseMat[sparseMat->idx[i] * sparseMat->M + row] = sparseMat->vals[i];
	}
}

static void 
printMatCSR(char *name, MatCSR *mat)
{
	#ifdef DISPLAY
		printf("%s:\n", name);
		printf("Ptr : ");
		for (int i = 0; i < mat->M+1; i++)
			printf("%d ", mat->ptrs[i]);
		
		printf("\nIdx : ");
		for (int i = 0; i < mat->ptrs[mat->M]; i++)
			printf("%d ", mat->idx[i]);

		printf("\nVals : ");
		for (int i = 0; i < mat->ptrs[mat->M]; i++)
			printf("%.2f ", mat->vals[i]);

		printf("\n");
	#endif
}

static void
printDenseMat(const char *name, int M, int N, float *mat)
{
	#ifdef DISPLAY
		printf("%s:\n", name);
		for (int row = 0; row < M; row++)
		{
			for (int col = 0; col < N; col++)
			{
				printf("%.2f ", mat[col * M + row]);
			}
			printf("\n");
		}
	#endif
}

static float norm2(int N, float *a, float *b)
{
    float res = 0;
    for (int i = 0; i < N; i++)
    {
        res += fabsf(a[i] - b[i]);
    }
    return sqrtf(res);
}

void testDenseCublasGemv(int N, float *denseMat, float *x, float *y)
{
    float *dDenseMat;
    float *dX;
    float *dY;
    cudaMalloc(&dDenseMat, N*N*sizeof(float));
    cudaMalloc(&dX, N*sizeof(float));
    cudaMalloc(&dY, N*sizeof(float));

    cublasHandle_t  handle;
    cublasCreate(&handle);
    cublasSetMatrix(N, N, sizeof(float), denseMat, N, dDenseMat, N);
    cublasSetVector(N, sizeof(float), x, 1, dX, 1);
//    cublasSetVector(N, sizeof(float), y, 1, dY, 1);
    float al=1.0f;
    float bet =0.0f;
    cublasSgemv(handle, CUBLAS_OP_N, N, N, &al, dDenseMat, N, dX, 1, &bet, dY, 1);
    cublasGetVector(N, sizeof(float), dY, 1, y, 1);

    cudaFree(dDenseMat);
    cudaFree(dX);
    cudaFree(dY);
}

void testSparseGPU1(int N, MatCSR *sparseMat, float *x, float *y)
{
    float *dX, *dY, *dVals;
    int *dPtrs, *dIdx;
    cudaMalloc(&dPtrs, (N+1)*sizeof(int));
    cudaMalloc(&dIdx, sparseMat->ptrs[N]*sizeof(int));
    cudaMalloc(&dVals, sparseMat->ptrs[N]*sizeof(float));
    cudaMalloc(&dX, N*sizeof(float));
    cudaMalloc(&dY, N*sizeof(float));

    cudaMemcpy(dPtrs, sparseMat->ptrs, (N+1)*sizeof(int), cudaMemcpyHostToDevice);
    cudaMemcpy(dIdx, sparseMat->idx, sparseMat->ptrs[N]*sizeof(int), cudaMemcpyHostToDevice);
    cudaMemcpy(dVals, sparseMat->vals, sparseMat->ptrs[N]*sizeof(float), cudaMemcpyHostToDevice);
    cudaMemcpy(dX, x, N*sizeof(float), cudaMemcpyHostToDevice);

    sparseGemvGPU<<<10, N/10>>>(dY, dPtrs, dIdx, dVals, dX);
    cudaMemcpy(y, dY, N*sizeof(float), cudaMemcpyDeviceToHost);

    cudaFree(dPtrs);
    cudaFree(dIdx);
    cudaFree(dVals);
    cudaFree(dX);
    cudaFree(dY);
}

void testSparseGPU2(int N, MatCSR *sparseMat, float *x, float *y)
{
    float *dX, *dY, *dVals;
    int *dPtrs, *dIdx;
    cudaMalloc(&dPtrs, (N+1)*sizeof(int));
    cudaMalloc(&dIdx, sparseMat->ptrs[N]*sizeof(int));
    cudaMalloc(&dVals, sparseMat->ptrs[N]*sizeof(float));
    cudaMalloc(&dX, N*sizeof(float));
    cudaMalloc(&dY, N*sizeof(float));

    cudaMemcpy(dPtrs, sparseMat->ptrs, (N+1)*sizeof(int), cudaMemcpyHostToDevice);
    cudaMemcpy(dIdx, sparseMat->idx, sparseMat->ptrs[N]*sizeof(int), cudaMemcpyHostToDevice);
    cudaMemcpy(dVals, sparseMat->vals, sparseMat->ptrs[N]*sizeof(float), cudaMemcpyHostToDevice);
    cudaMemcpy(dX, x, N*sizeof(float), cudaMemcpyHostToDevice);

    sparseGemvGPU2<<<10, N/10>>>(dY, dPtrs, dIdx, dVals, dX);
    cudaMemcpy(y, dY, N*sizeof(float), cudaMemcpyDeviceToHost);

    cudaFree(dPtrs);
    cudaFree(dIdx);
    cudaFree(dVals);
    cudaFree(dX);
    cudaFree(dY);
}

int main(int argc, char const *argv[])
{
	srand(42);

	int N = 100;
	MatCSR sparseMat;
	float *denseMat = (float*) malloc(N * N * sizeof(float));
	float *x = (float *) malloc(N * sizeof(float));
    float *y = (float *) malloc(N * sizeof(float));
    float *yRef = (float *) malloc(N * sizeof(float));
	for (int i = 0; i < N; i++)
		x[i] = i+1;
	struct timeval start, stop;

	initRandomMatCSR(&sparseMat, N, N, 0.1, -20, 20);
	sparseToDense(denseMat, &sparseMat);
	printMatCSR("A sparse", &sparseMat);
	printDenseMat("A dense", N, N, denseMat);
	printDenseMat("x", 10, 1, x);

	printf("CPU:\n");
    gettimeofday(&start, NULL);
    sparseGemv(y, &sparseMat, x);
    gettimeofday(&stop, NULL);
	printDenseMat("y", N, 1, y);
	printf("duration = %e s\n", stop.tv_sec - start.tv_sec + 0.000001 * (stop.tv_usec - start.tv_usec));

    memcpy(yRef, y, N * sizeof(float));

	printf("GPU with CUblas :\n");
	gettimeofday(&start, NULL);
    testDenseCublasGemv(N, denseMat, x, y);
    gettimeofday(&stop, NULL);
	printDenseMat("y", N, 1, y);
	printf("duration = %e s\n", stop.tv_sec - start.tv_sec + 0.000001 * (stop.tv_usec - start.tv_usec));
    printf("error = %e s\n", norm2(N, y, yRef));

    printf("GPU custom sparse implementation 1:\n");
    gettimeofday(&start, NULL);
    testSparseGPU1(N, &sparseMat, x, y);
    gettimeofday(&stop, NULL);
    printDenseMat("y", N, 1, y);
    printf("duration = %e s\n", stop.tv_sec - start.tv_sec + 0.000001 * (stop.tv_usec - start.tv_usec));
    printf("error = %e s\n", norm2(N, y, yRef));

    printf("GPU custom sparse implementation 2:\n");
    gettimeofday(&start, NULL);
    testSparseGPU2(N, &sparseMat, x, y);
    gettimeofday(&stop, NULL);
    printDenseMat("y", N, 1, y);
    printf("duration = %e s\n", stop.tv_sec - start.tv_sec + 0.000001 * (stop.tv_usec - start.tv_usec));
    printf("error = %e s\n", norm2(N, y, yRef));


    freeMatCSR(&sparseMat);
	free(denseMat);
	free(x);
	free(y);
}