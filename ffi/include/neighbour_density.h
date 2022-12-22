#ifndef GRAV_POT_H
#define GRAV_POT_H

#if HAVE_CL_OPENCL_H == 1
#include "ocl.h"
extern cl_program ocl_program_grav_pot;
extern cl_kernel ocl_kernel_grav_pot;
int ocl_init_grav_pot(void);
void ocl_free_grav_pot(void);
#endif

void neighbour_density_omp(float *m,
	   					   float *x1,
						   float *x2,
						   float *x3,
						   float *neighbour_density_n,
						   float *neighbour_density_m,
						   int n_neigh,
						   int n_tot,
						   int *n_procs);
/*double grav_pot_ocl(float *m, float *x1, float *x2, float *x3, float *EPOT, int n);
double grav_pot_unthreaded(float *m, float *x1, float *x2, float *x3, float *EPOT, int n);*/

#endif