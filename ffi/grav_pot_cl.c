#define CL_TARGET_OPENCL_VERSION 200
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <CL/opencl.h>

 
// OpenCL kernel. Each work item takes care of one element of c
/*const char *kernelSource =                                       "\n" \
"#pragma OPENCL EXTENSION cl_khr_fp64 : enable                    \n" \
"__kernel void grav_pot_kernel(  __global double *m,              \n" \
"                                __global double *x1,             \n" \
"                                __global double *x2,             \n" \
"                                __global double *x3,             \n" \
"                                __global double *EPOT,           \n" \
"                                __global int *n)                 \n" \
"{                                                               \n" \
"    //Get our global thread ID                                  \n" \
"    int id = get_global_id(0);                                  \n" \
"                                                                \n" \
"    //Make sure we do not go out of bounds                      \n" \
"    if (id < n) {                                               \n" \
"        c[id] = a[id] + b[id];                                  \n" \
"    }                                                           \n" \
"}*/                                                               \n" \
                                                                "\n" ;

double grav_pot(double *m,
                double *x1,
                double *x2,
                double *x3,
                double *EPOT,
                int n,
                )
{
    cl_mem l_m;
    cl_mem l_x1;
    cl_mem l_x2;
    cl_mem l_x3;
    cl_mem l_EPOT;
    
    cl_platform_id cpPlatform;        // OpenCL platform
    cl_device_id device_id;           // device ID
    cl_context context;               // context
    cl_command_queue queue;           // command queue
    cl_program program;               // program
    cl_kernel kernel;                 // kernel
    
    size_t bytes = n*sizeof(double);
    
    size_t globalSize, localSize;
    cl_int err;
 
    // Number of work items in each local work group
    localSize = 256;
 
    // Number of total work items - localSize must be devisor
    //globalSize = ceil(n/(float)localSize)*localSize;
    globalSize = (((int) (n/localSize)) + 1)*localSize;
    
    // Bind to platform
    err = clGetPlatformIDs(1, &cpPlatform, NULL);
 
    // Get ID for the device
    err = clGetDeviceIDs(cpPlatform, CL_DEVICE_TYPE_GPU, 1, &device_id, NULL);
 
    // Create a context 
    context = clCreateContext(0, 1, &device_id, NULL, NULL, &err);
 
    // Create a command queue
    queue = clCreateCommandQueueWithProperties(context, device_id, 0, &err);
 
    // Create the compute program from the source buffer
    program = clCreateProgramWithSource(context, 1,
                            (const char **) & kernelSource, NULL, &err);
 
    // Build the program executable
    clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
 
    // Create the compute kernel in the program we wish to run
    kernel = clCreateKernel(program, "vecAdd", &err);
 
    // Create the input and output arrays in device memory for our calculation
    l_m = clCreateBuffer(context, CL_MEM_READ_ONLY, bytes, NULL, NULL);
    l_x1 = clCreateBuffer(context, CL_MEM_READ_ONLY, bytes, NULL, NULL);
    l_x2 = clCreateBuffer(context, CL_MEM_READ_ONLY, bytes, NULL, NULL);
    l_x3 = clCreateBuffer(context, CL_MEM_READ_ONLY, bytes, NULL, NULL);
    l_EPOT = clCreateBuffer(context, CL_MEM_WRITE_ONLY, bytes, NULL, NULL);
    
    err = clEnqueueWriteBuffer(queue, l_m, CL_TRUE, 0,
                                   bytes, m, 0, NULL, NULL);
    err |= clEnqueueWriteBuffer(queue, l_x1, CL_TRUE, 0,
                                   bytes, x1, 0, NULL, NULL);
    err |= clEnqueueWriteBuffer(queue, l_x2, CL_TRUE, 0,
                                   bytes, x2, 0, NULL, NULL);
    err |= clEnqueueWriteBuffer(queue, l_x3, CL_TRUE, 0,
                                   bytes, x3, 0, NULL, NULL);
    err |= clEnqueueWriteBuffer(queue, l_EPOT, CL_TRUE, 0,
                                   bytes, EPOT, 0, NULL, NULL);
    
    // Set the arguments to our compute kernel
    err  = clSetKernelArg(kernel, 0, sizeof(cl_mem), &l_m);
    err |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &l_x1);
    err |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &l_x2);
    err |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &l_x3);
    err |= clSetKernelArg(kernel, 2, sizeof(cl_mem), &l_EPOT);
    err |= clSetKernelArg(kernel, 3, sizeof(int), &n);
 
    // Execute the kernel over the entire range of the data set 
    err = clEnqueueNDRangeKernel(queue, kernel, 1, NULL, &globalSize, &localSize,
                                                              0, NULL, NULL);
 
    // Wait for the command queue to get serviced before reading back results
    clFinish(queue);
 
    // Read the results from the device
    clEnqueueReadBuffer(queue, l_EPOT, CL_TRUE, 0,
                                bytes, EPOT, 0, NULL, NULL );
 
    //Sum up vector c and print result divided by n, this should equal 1 within error
 
    // release OpenCL resources
    clReleaseMemObject(l_m);
    clReleaseMemObject(l_x1);
    clReleaseMemObject(l_x2);
    clReleaseMemObject(l_x3);
    clReleaseMemObject(l_EPOT);
    clReleaseProgram(program);
    clReleaseKernel(kernel);
    clReleaseCommandQueue(queue);
    clReleaseContext(context);

}

/*int main( int argc, char* argv[] )
{
    // Length of vectors
    unsigned int n = 20000000;
 
    // Host input vectors
    double *h_a;
    double *h_b;
    // Host output vector
    double *h_c;
 
    // Device input buffers
    cl_mem d_a;
    cl_mem d_b;
    // Device output buffer
    cl_mem d_c;
 
    cl_platform_id cpPlatform;        // OpenCL platform
    cl_device_id device_id;           // device ID
    cl_context context;               // context
    cl_command_queue queue;           // command queue
    cl_program program;               // program
    cl_kernel kernel;                 // kernel
 
    // Size, in bytes, of each vector
    size_t bytes = n*sizeof(double);
 
    // Allocate memory for each vector on host
    h_a = (double*)malloc(bytes);
    h_b = (double*)malloc(bytes);
    h_c = (double*)malloc(bytes);
 
    // Initialize vectors on host
    int i;
    for( i = 0; i < n; i++ )
    {
        h_a[i] = sinf(i)*sinf(i);
        h_b[i] = cosf(i)*cosf(i);
    }
 
    size_t globalSize, localSize;
    cl_int err;
 
    // Number of work items in each local work group
    localSize = 256;
 
    // Number of total work items - localSize must be devisor
    //globalSize = ceil(n/(float)localSize)*localSize;
    globalSize = (((int) (n/localSize)) + 1)*localSize;

    printf("localSize %d; globalSize %d\n", localSize,globalSize);
 
    // Bind to platform
    err = clGetPlatformIDs(1, &cpPlatform, NULL);
 
    // Get ID for the device
    err = clGetDeviceIDs(cpPlatform, CL_DEVICE_TYPE_GPU, 1, &device_id, NULL);
 
    // Create a context 
    context = clCreateContext(0, 1, &device_id, NULL, NULL, &err);
 
    // Create a command queue
    queue = clCreateCommandQueueWithProperties(context, device_id, 0, &err);
 
    // Create the compute program from the source buffer
    program = clCreateProgramWithSource(context, 1,
                            (const char **) & kernelSource, NULL, &err);
 
    // Build the program executable
    clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
 
    // Create the compute kernel in the program we wish to run
    kernel = clCreateKernel(program, "vecAdd", &err);
 
    // Create the input and output arrays in device memory for our calculation
    d_a = clCreateBuffer(context, CL_MEM_READ_ONLY, bytes, NULL, NULL);
    d_b = clCreateBuffer(context, CL_MEM_READ_ONLY, bytes, NULL, NULL);
    d_c = clCreateBuffer(context, CL_MEM_WRITE_ONLY, bytes, NULL, NULL);
 
    // Write our data set into the input array in device memory
    err = clEnqueueWriteBuffer(queue, d_a, CL_TRUE, 0,
                                   bytes, h_a, 0, NULL, NULL);
    err |= clEnqueueWriteBuffer(queue, d_b, CL_TRUE, 0,
                                   bytes, h_b, 0, NULL, NULL);
 
    // Set the arguments to our compute kernel
    err  = clSetKernelArg(kernel, 0, sizeof(cl_mem), &d_a);
    err |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &d_b);
    err |= clSetKernelArg(kernel, 2, sizeof(cl_mem), &d_c);
    err |= clSetKernelArg(kernel, 3, sizeof(unsigned int), &n);
 
    // Execute the kernel over the entire range of the data set 
    err = clEnqueueNDRangeKernel(queue, kernel, 1, NULL, &globalSize, &localSize,
                                                              0, NULL, NULL);
 
    // Wait for the command queue to get serviced before reading back results
    clFinish(queue);
 
    // Read the results from the device
    clEnqueueReadBuffer(queue, d_c, CL_TRUE, 0,
                                bytes, h_c, 0, NULL, NULL );
 
    //Sum up vector c and print result divided by n, this should equal 1 within error
    double sum = 0;
    for(i=0; i<n; i++)
        sum += h_c[i];
    printf("final result: %f\n", sum/n);
 
    // release OpenCL resources
    clReleaseMemObject(d_a);
    clReleaseMemObject(d_b);
    clReleaseMemObject(d_c);
    clReleaseProgram(program);
    clReleaseKernel(kernel);
    clReleaseCommandQueue(queue);
    clReleaseContext(context);
 
    //release host memory
    free(h_a);
    free(h_b);
    free(h_c);
 
    return 0;
}*/
