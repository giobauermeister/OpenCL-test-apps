//************************************************************
// Demo OpenCL application to compute a simple vector addition
// computation between 2 arrays on the GPU
// ************************************************************
#include <stdio.h>
#include <stdlib.h>
#include <CL/cl.h>
//
// OpenCL source code
const char* OpenCLSource[] = {
"__kernel void VectorAdd(__global int* c, __global int* a,__global int* b)",
"{",
" // Index of the elements to add \n",
" unsigned int n = get_global_id(0);",
" // Sum the nth element of vectors a and b and store in c \n",
" c[n] = a[n] + b[n];",
"}"
};
// Some interesting data for the vectors
int InitialData1[20] = {37,50,54,50,56,0,43,43,74,71,32,36,16,43,56,100,50,25,15,17};
int InitialData2[20] = {35,51,54,58,55,32,36,69,27,39,35,40,16,44,55,14,58,75,18,15};
// Number of elements in the vectors to be added
#define SIZE 100
// Main function
// ************************************************************
int main(int argc, char **argv)
{
     // Two integer source vectors in Host memory
     int HostVector1[SIZE], HostVector2[SIZE];
     //Output Vector
     int HostOutputVector[SIZE];
     // Initialize with some interesting repeating data
     for(int c = 0; c < SIZE; c++)
     {
          HostVector1[c] = InitialData1[c%20];
          HostVector2[c] = InitialData2[c%20];
          HostOutputVector[c] = 0;
     }
     //Get an OpenCL platform
     cl_platform_id cpPlatform;
     clGetPlatformIDs(1, &cpPlatform, NULL);
     // Get a GPU device
     cl_device_id cdDevice;
     clGetDeviceIDs(cpPlatform, CL_DEVICE_TYPE_GPU, 1, &cdDevice, NULL);
     char cBuffer[1024];
     clGetDeviceInfo(cdDevice, CL_DEVICE_NAME, sizeof(cBuffer), &cBuffer, NULL);
     printf("CL_DEVICE_NAME: %s\n", cBuffer);
     clGetDeviceInfo(cdDevice, CL_DRIVER_VERSION, sizeof(cBuffer), &cBuffer, NULL);
     printf("CL_DRIVER_VERSION: %s\n\n", cBuffer);
     // Create a context to run OpenCL enabled GPU
     cl_context GPUContext = clCreateContextFromType(0, CL_DEVICE_TYPE_GPU, NULL, NULL, NULL);
     // Create a command-queue on the GPU device
     cl_command_queue cqCommandQueue = clCreateCommandQueue(GPUContext, cdDevice, 0, NULL);
     // Allocate GPU memory for source vectors AND initialize from CPU memory
     cl_mem GPUVector1 = clCreateBuffer(GPUContext, CL_MEM_READ_ONLY |
     CL_MEM_COPY_HOST_PTR, sizeof(int) * SIZE, HostVector1, NULL);
     cl_mem GPUVector2 = clCreateBuffer(GPUContext, CL_MEM_READ_ONLY |
     CL_MEM_COPY_HOST_PTR, sizeof(int) * SIZE, HostVector2, NULL);
     // Allocate output memory on GPU
     cl_mem GPUOutputVector = clCreateBuffer(GPUContext, CL_MEM_WRITE_ONLY,
     sizeof(int) * SIZE, NULL, NULL);
     // Create OpenCL program with source code
     cl_program OpenCLProgram = clCreateProgramWithSource(GPUContext, 7, OpenCLSource, NULL, NULL);
     // Build the program (OpenCL JIT compilation)
     clBuildProgram(OpenCLProgram, 0, NULL, NULL, NULL, NULL);
     // Create a handle to the compiled OpenCL function (Kernel)
     cl_kernel OpenCLVectorAdd = clCreateKernel(OpenCLProgram, "VectorAdd", NULL);
     // In the next step we associate the GPU memory with the Kernel arguments
     clSetKernelArg(OpenCLVectorAdd, 0, sizeof(cl_mem), (void*)&GPUOutputVector);
     clSetKernelArg(OpenCLVectorAdd, 1, sizeof(cl_mem), (void*)&GPUVector1);
     clSetKernelArg(OpenCLVectorAdd, 2, sizeof(cl_mem), (void*)&GPUVector2);
     // Launch the Kernel on the GPU
     // This kernel only uses global data
     size_t WorkSize[1] = {SIZE}; // one dimensional Range
     clEnqueueNDRangeKernel(cqCommandQueue, OpenCLVectorAdd, 1, NULL,
     WorkSize, NULL, 0, NULL, NULL);
     // Copy the output in GPU memory back to CPU memory
     clEnqueueReadBuffer(cqCommandQueue, GPUOutputVector, CL_TRUE, 0,
     SIZE * sizeof(int), HostOutputVector, 0, NULL, NULL);
     // Cleanup
     clReleaseKernel(OpenCLVectorAdd);
     clReleaseProgram(OpenCLProgram);
     clReleaseCommandQueue(cqCommandQueue);
     clReleaseContext(GPUContext);
     clReleaseMemObject(GPUVector1);
     clReleaseMemObject(GPUVector2);
     clReleaseMemObject(GPUOutputVector);
     for( int i =0 ; i < SIZE; i++)
          printf("[%d + %d = %d]\n",HostVector1[i], HostVector2[i], HostOutputVector[i]);
     return 0;
}
