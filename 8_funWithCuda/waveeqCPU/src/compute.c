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

int compute_wave(double *unew,double *ucur,double *uold,int nx,int ny,double dx,double dy,double dt,int niter,double cel) {

	int i,j,iter;
	int index;

	index = 1;

	for(iter=0;iter<niter;iter++) {
		for (i=1; i<nx-1; i++) {
			for (j=1; j<ny-1; j++) {
				unew[i*ny+j] =2.0*ucur[i*ny+j]-uold[i*ny+j]
						+cel*cel*dt*dt*((ucur[(i-1)*ny+j]
						-2.0*ucur[i*ny+j]+ucur[(i+1)*ny+j])/(dx*dx)
						+(ucur[i*ny+(j-1)]
						-2.0*ucur[i*ny+j]
						+ucur[i*ny+(j+1)])/(dy*dy));
			}
		}

		/* Reflecting Conditions */
		for (j=0; j<ny; j++) unew[(nx-1)*ny+j] = ucur[(nx-3)*ny+j];
		for (j=0; j<ny; j++) unew[j] = ucur[2*ny+j];
		for (i=0; i<nx; i++) unew[i*ny] = ucur[i*ny+2];
		for (i=0; i<nx; i++) unew[i*ny+ny-1] = ucur[i*ny+ny-3]; 

		memcpy(uold, ucur, nx*ny*sizeof(double));
		memcpy(ucur, unew, nx*ny*sizeof(double));

		if (index%100 == 0) vtkprint(index,nx,ny,dx,dy,ucur);

		index++;
	}

	return EXIT_SUCCESS;

}

