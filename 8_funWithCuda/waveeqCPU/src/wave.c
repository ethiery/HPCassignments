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
#include <math.h>
#include <sys/time.h>

#define _max(a,b) (a>b)?a:b;

void usage(char * argv[]);
int vtkprint(int index,int nx, int ny,double dx, double dy,double *uold);
int compute_wave(double *unew,double *ucur,double *uold,int nx,int ny,double dx,double dy,double dt,int niter,double cel);

int main(int argc,char* argv[]) {

	int i,j;
	int sizex, sizey, nx, ny, niter;
	double x0,y0;
	double dt, dx, dy;
	const double sigma = 0.2;
	double *unew, *ucur, *uold;

	const double cel = 4.0;

	struct timeval t1;
	struct timeval t2;
	double time;

	if(argc < 7) usage(argv);

/* Struct definition */

	sizex = atoi(argv[1]);
	sizey = atoi(argv[2]);
	nx = atoi(argv[3]);
	ny = atoi(argv[4]);
	dt = (double) atof(argv[5]);
	niter = atoi(argv[6]);

	dx = (double) sizex / (double) nx;
	dy = (double) sizey / (double) ny;

	fprintf(stdout,"dx = %g and dy = %g \n",dx,dy);
	fprintf(stdout,"dt = %g \n",dt);

/* Memory allocation */

	unew = (double *)calloc(nx*ny,sizeof(double));
	ucur = (double *)calloc(nx*ny,sizeof(double));
	uold = (double *)calloc(nx*ny,sizeof(double));

/* Initialization */

	x0 = (double) ( nx ) / 2.0 ;
	y0 = (double) ( ny ) / 4.0 ;

	printf("x0 = %g -- y0 = %g \n",x0,y0);
	printf("nx = %i -- ny = %i \n",nx,ny);
	printf("CFL = cel * dt / dx = %g\n",cel*dt/dx);
	
	for (i=0; i<nx; i++) {
		for (j=0; j<ny; j++) {
			ucur[i*ny+j] = _max(0.0,1.0-1.0/(sigma*sqrt(6.3))*exp(-0.5*((double)((i-x0)*(i-x0)*dx*dx)/(sigma*sigma)+(double)((j-y0)*(j-y0)*dy*dy)/(sigma*sigma))) );
		}
	}
	memcpy(uold, ucur, nx*ny*sizeof(double));
	vtkprint(0,nx,ny,dx,dy,ucur);

/* Wave propagation */
	gettimeofday(&t1, NULL);
	compute_wave(unew,ucur,uold,nx,ny,dx,dy,dt,niter,cel);
	gettimeofday(&t2, NULL);
	time =( (t2.tv_sec-t1.tv_sec) + (t2.tv_usec-t1.tv_usec)*1.0e-06 );
	printf("time to compute stencil   = %g s \n", time);

/* Memory free */

	free(unew);
	free(ucur);
	free(uold);
	return EXIT_SUCCESS;

}


