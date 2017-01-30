#include <stdio.h>
#include <stdlib.h>

__global__ void VecAdd(float *A, float *B, float *C)
{
	int i = threadIdx.x; 
	C[i] = A[i] + B[i];
}

void printVec(int N, float *vec)
{
	for (int i = 0; i < N; i++)
	{
		printf("%.2f ", vec[i]);
	}
	printf("\n");
}

int main(int argc, char const *argv[])
{
	int deviceCount, device; 
	cudaGetDeviceCount(&deviceCount); 

	for (device = 0; device < deviceCount; ++device) 
	{ 
		cudaDeviceProp deviceProp; 
		cudaGetDeviceProperties(&deviceProp, device); 
		printf("Device %d (%s) has %d multiprocessors, and warps of size %d\n", 
			device, deviceProp.name, deviceProp.multiProcessorCount, deviceProp.warpSize);
	}

	int N = 50000;
	size_t size = N * sizeof(float);

	float *hA = (float *)malloc(size);
	float *hB = (float *)malloc(size);
	float *hC = (float *)malloc(size);

	for (int i = 0; i < N; i++)
	{
		hA[i] = i;
		hB[i] = -2*i;
	}

	printVec(10, hA);
	printf("+\n");
	printVec(10, hB);

	float *dA, *dB, *dC;
	cudaMalloc(&dA, size);
	cudaMalloc(&dB, size);
	cudaMalloc(&dC, size);

	cudaMemcpy(dA, hA, size, cudaMemcpyHostToDevice);
	cudaMemcpy(dB, hB, size, cudaMemcpyHostToDevice);

	VecAdd<<<4, N/4>>>(dA, dB, dC);

	cudaMemcpy(hC, dC, size, cudaMemcpyDeviceToHost);

	printf("=\n");
	printVec(10, hC);

	cudaFree(dA);
    cudaFree(dB);
    cudaFree(dC);

    free(hA);
    free(hB);
    free(hC);
}