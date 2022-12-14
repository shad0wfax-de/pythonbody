#ifndef CUMMEAN_H
#define CUMMEAN_H

#if HAVE_CL_OPENCL_H == 1
#include "ocl.h"

extern cl_program ocl_program_cummean;
extern cl_kernel ocl_kernel_cummean;
int ocl_init_cummean(void);
void ocl_free_cummean(void);
#endif

double cummean_omp(float *target, float *source, int n);
double cummean_ocl(float *target, float *source, int n);
double cummean_unthreaded(float *target, float *source, int n);


#endif
