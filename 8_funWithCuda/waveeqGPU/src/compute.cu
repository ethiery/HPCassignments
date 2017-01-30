/*This file is part of waveeq project.

  waveeq project is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  waveeq project is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with waveeq project.  If not, see <http://www.gnu.org/licenses/>.
  */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef BLOCK_SIZE
#define BLOCK_SIZE 10
#endif

int compute_wave_cpu(double *unew,double *ucur,double *uold,int nx,int ny,double dx,double dy,double dt,int niter,double cel)
{
	for(int iter = 0; iter < niter; iter++)
    {
		for (int i = 1; i < nx-1; i++)
			for (int j = 1; j < ny-1; j++)
            {
				unew[i*ny+j] =2.0*ucur[i*ny+j]-uold[i*ny+j]
						+cel*cel*dt*dt*((ucur[(i-1)*ny+j] - 2.0*ucur[i*ny+j] + ucur[(i+1)*ny+j])/(dx*dx)
                                        +(ucur[i*ny+(j-1)] - 2.0*ucur[i*ny+j] + ucur[i*ny+(j+1)])/(dy*dy));
			}

		/* Reflecting Conditions */
		for (int j = 0; j < ny; j++) unew[(nx-1)*ny+j] = ucur[(nx-3)*ny+j];
		for (int j = 0; j < ny; j++) unew[j] = ucur[2*ny+j];
		for (int i = 0; i < nx; i++) unew[i*ny] = ucur[i*ny+2];
		for (int i = 0; i < nx; i++) unew[i*ny+ny-1] = ucur[i*ny+ny-3];

		memcpy(uold, ucur, nx*ny*sizeof(double));
		memcpy(ucur, unew, nx*ny*sizeof(double));
	}

	return EXIT_SUCCESS;
}

__global__ void compute_wave_gpu(double * __restrict__ unew, const double * __restrict__ ucur,
                                 const double * __restrict__ uold, int nx, int ny,
                                 double dx, double dy, double dt,double cel)
{
    int x = blockIdx.x * blockDim.x + threadIdx.x;
    int y = blockIdx.y * blockDim.y + threadIdx.y;

    int inner = (0 < x && x < nx - 1 && 0 < y && y < ny - 1);

    if (inner)
        unew[x * ny + y] = 2.0 * ucur[x*ny+y] - uold[x*ny+y]
                           + cel*cel*dt*dt *
                             ((ucur[(x-1)*ny+y] - 2.0*ucur[x*ny+y] + ucur[(x+1)*ny+y]) / (dx*dx)
                              + (ucur[x*ny+(y-1)] - 2.0*ucur[x*ny+y] + ucur[x*ny+(y+1)]) / (dy*dy));

    /* Reflecting Conditions */
    if (x == (nx - 1)) unew[(nx - 1) * ny + y] = ucur[(nx - 3) * ny + y];
    if (x == 0) unew[y] = ucur[2 * ny + y];
    if (y == 0) unew[x * ny] = ucur[x * ny + 2];
    if (y == (ny - 1)) unew[x * ny + ny - 1] = ucur[x * ny + ny - 3];
}

__global__ void compute_wave_gpu2(double * __restrict__ unew, const double * __restrict__ ucur,
                                 const double * __restrict__ uold, int nx, int ny,
                                 double dx, double dy, double dt,double cel)
{
    int x = blockIdx.x * blockDim.x + threadIdx.x;
    int y = blockIdx.y * blockDim.y + threadIdx.y;

    // copy block in sm
    __shared__ double s_ucur[BLOCK_SIZE + 2][BLOCK_SIZE + 2];
    s_ucur[threadIdx.x + 1][threadIdx.y + 1] = ucur[x * ny + y];

    // copy halo in sm too
    if (threadIdx.x == 0 && 0 < x)                   s_ucur[0][threadIdx.y + 1] = ucur[(x-1) * ny + y];
    if (threadIdx.x == blockDim.x - 1 && x < nx-1)   s_ucur[BLOCK_SIZE + 1][threadIdx.y + 1] = ucur[(x+1) * ny + y];
    if (threadIdx.y == 0 && 0 < y)                   s_ucur[threadIdx.x + 1][0] = ucur[x * ny + (y-1)];
    if (threadIdx.y == blockDim.y - 1 && y < ny-1)   s_ucur[threadIdx.x + 1][BLOCK_SIZE + 1] = ucur[x * ny + (y+1)];

    // Synchronize to make sure the sub-matrices are loaded
    // before starting the computation
    __syncthreads();

    if (0 < x && x < nx-1 && 0 < y && y < ny-1)
        unew[x*ny+y] = 2.0 * s_ucur[1 + threadIdx.x][1 + threadIdx.y] - uold[x*ny+y]
                       +cel*cel*dt*dt*((s_ucur[threadIdx.x][1 + threadIdx.y] - 2.0*s_ucur[1 + threadIdx.x][1 + threadIdx.y] + s_ucur[2 + threadIdx.x][1 + threadIdx.y])/(dx*dx)
                                       +(s_ucur[1 + threadIdx.x][threadIdx.y] - 2.0*s_ucur[1 + threadIdx.x][1 + threadIdx.y] + s_ucur[1 + threadIdx.x][2 + threadIdx.y])/(dy*dy));

    /* Reflecting Conditions */
    if (x == 0) unew[y] = s_ucur[3][1 + threadIdx.y];
    if (y == 0) unew[x*ny] = s_ucur[1 + threadIdx.x][3];
    if (x == (nx-1)) unew[(nx-1)*ny+y] = s_ucur[BLOCK_SIZE - 1][1 + threadIdx.y];
    if (y == (ny-1)) unew[x*ny+(ny-1)] = s_ucur[1 + threadIdx.x][BLOCK_SIZE - 1];
}