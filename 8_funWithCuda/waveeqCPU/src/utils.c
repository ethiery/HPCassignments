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

#include <stdlib.h>
#include <stdio.h>

void usage(char * argv[]){
	fprintf(stderr,"Usage  %s sizex sizey nx ny dt iter\n", argv[0]);
	fprintf(stderr, "\tsizex    Domain size in the x coordinate\n");
	fprintf(stderr, "\tsizey    Domain size in the y coordinate\n");
	fprintf(stderr, "\tnx       Number of discretization in the x coordinate\n");
	fprintf(stderr, "\tny       Number of discretization in the y coordinate\n");
	fprintf(stderr, "\tdt       Time discretization\n");
	fprintf(stderr, "\tniter    Number of time iteration\n");
	exit(EXIT_FAILURE);
}

int vtkprint(int index,int nx, int ny,double dx, double dy,double *uold) {

	char file[120];
	FILE *fid;

	int i,j;

	sprintf (file,"sol/sol_%05d.vtk", index);
        fid = fopen(file,"w");
        if(fid == NULL) {
                fprintf(stderr,"Unable to open file %s\n",file);
                return EXIT_FAILURE;
        }


        fprintf(fid, "# vtk DataFile Version 2.0");
        fprintf(fid, "\n");
        fprintf(fid, "one scalar variable on a rectilinear grid");
        fprintf(fid, "\n");
        fprintf(fid, "ASCII");
        fprintf(fid, "\n");
        fprintf(fid, "DATASET RECTILINEAR_GRID");
        fprintf(fid, "\n");
        fprintf(fid, "DIMENSIONS %i %i %i",nx,ny,1);
        fprintf(fid, "\n");
        fprintf(fid, "X_COORDINATES %i float",nx);
        fprintf(fid, "\n");
        for (i=0; i<nx; i++) {
                fprintf(fid, "%e\t",(double)i * dx);
        }
        fprintf(fid, "\n");
        fprintf(fid, "Y_COORDINATES %i float",ny);
        fprintf(fid, "\n");
        for (j=0; j<ny; j++) {
                fprintf(fid, "%e\t",(double)j * dy);
        }
        fprintf(fid, "\n");
        fprintf(fid, "Z_COORDINATES %i float",1);
        fprintf(fid, "\n");
        fprintf(fid, "%e\t",0.0);
        fprintf(fid, "\n");
        fprintf(fid, "POINT_DATA %i",nx*ny);
        fprintf(fid, "\n");
        fprintf(fid, "SCALARS scalar_variable float 1");
        fprintf(fid, "\n");
        fprintf(fid, "LOOKUP_TABLE default");
        fprintf(fid, "\n");

        for (i=0; i<nx; i++) {
                for (j=0; j<ny; j++) {
                        fprintf(fid, "%g",uold[i*ny+j]);
			fprintf(fid, "\t");
                }
        }

        fclose(fid);

	return EXIT_SUCCESS;
}

