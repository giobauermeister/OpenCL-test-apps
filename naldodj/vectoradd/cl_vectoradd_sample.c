//************************************************************
// Demo OpenCL application to compute a simple vector addition
// computation between 2 arrays on the GPU
// ************************************************************
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <CL/cl.h>

// OpenCL source code
const char* OpenCLSource[] = {
"__kernel void VectorAdd(__global int* iVRet1,"
"                        __global int* iVRet2,"
"                        constant int* restrict iBase,"
"                        __global int* iVGet1,"
"                        __global int* iVGet2)",
"{",
" ",
" unsigned long long int n = get_global_id(0);",
" int s  = (iVGet1[n] + iVGet2[n]);",
" int v  = (s >= iBase[0]) ? s%iBase[0] : s ;"
" int v1 = (s >= iBase[0]) ? s/iBase[0] : 0 ;"
" iVRet2[n-1] = v1 ; iVRet1[n] = v ;",
"}"
};

// Some interesting data for the vectors
// **** Keep the zero in left ****
int InitialData1[21] = {0,8,4,3,5,7,0,8,3,9,2,4,5,6,7,8,9,0,1,2,3};
int InitialData2[21] = {0,2,5,7,8,5,9,9,4,3,9,7,5,6,8,8,9,9,9,8,7};

// Number of elements in the vectors to be added
#define SIZE 21

// if SIZE == 21 then
//  084357083924567890123
//  025785994397568899987 (+)
//  ---------------------
//  110143078322136790110 (=)

int copy_array(int source_arr[], int target_arr[], int size)
{

    int *source_ptr = source_arr;
    int *target_ptr = target_arr;

    int *end_ptr = &source_arr[size - 1];

    while(source_ptr <= end_ptr)
    {
        *target_ptr = *source_ptr;
        source_ptr++;
        target_ptr++;
    }

}

bool KeepCalc(int source_arr[], int size)
{

    int *p1 = source_arr;
    int *p2 = source_arr + size - 1;

    while (p1 < p2) {
        if ( *p1++ || *p2-- )
        {
            return(true);
        }
    }
    return(false);
    
}

// Main function
// ************************************************************
int main(int argc, char **argv)
{
     // Two integer source vectors in Host memory
     int HostVector1[SIZE], HostVector2[SIZE], HostVectorB=10; 
     
     //Output Vector
     int HostOutputVector0[SIZE];
     int HostOutputVector1[SIZE];
     
     // Initialize with some interesting repeating data
     int c;
     for(c = 0; c < SIZE; c++)
     {
          HostVector1[c] = InitialData1[c%21];
          HostVector2[c] = InitialData2[c%21];
          HostOutputVector0[c] = 0;
          HostOutputVector1[c] = 0;
     }
     //Get an OpenCL platform
     cl_platform_id cpPlatform;
     clGetPlatformIDs(1, &cpPlatform, NULL);
     
     // Get a GPU device
     cl_device_id cdDevice;
     clGetDeviceIDs(cpPlatform, CL_DEVICE_TYPE_GPU, 1, &cdDevice, NULL);
     char cBuffer[1024];
     clGetDeviceInfo(cdDevice, CL_DEVICE_NAME, sizeof(cBuffer), &cBuffer, NULL);
     printf("\nCL_DEVICE_NAME: %s\n", cBuffer);
     clGetDeviceInfo(cdDevice, CL_DRIVER_VERSION, sizeof(cBuffer), &cBuffer, NULL);
     printf("CL_DRIVER_VERSION: %s\n\n", cBuffer);
     
     // Create a context to run OpenCL enabled GPU
     cl_context GPUContext = clCreateContextFromType(0, CL_DEVICE_TYPE_GPU, NULL, NULL, NULL);
     
     // Create a command-queue on the GPU device
     cl_command_queue cqCommandQueue = clCreateCommandQueue(GPUContext, cdDevice, 0, NULL);
     
     // Create OpenCL program with source code
     cl_program OpenCLProgram = clCreateProgramWithSource(GPUContext, 7, OpenCLSource, NULL, NULL);
     
     // Build the program (OpenCL JIT compilation)
     clBuildProgram(OpenCLProgram, 0, NULL, NULL, NULL, NULL);
     
     // Create a handle to the compiled OpenCL function (Kernel)
     cl_kernel OpenCLVectorAdd = clCreateKernel(OpenCLProgram, "VectorAdd", NULL);
     
     bool bKeekCalc = false;
     do 
     {

         // Allocate GPU memory for source vectors AND initialize from CPU memory
         cl_mem GPUVector1 = clCreateBuffer(GPUContext, CL_MEM_READ_ONLY |
         CL_MEM_COPY_HOST_PTR, sizeof(int) * SIZE, HostVector1, NULL);
         cl_mem GPUVector2 = clCreateBuffer(GPUContext, CL_MEM_READ_ONLY |
         CL_MEM_COPY_HOST_PTR, sizeof(int) * SIZE, HostVector2, NULL);
         cl_mem GPUVectorB = clCreateBuffer(GPUContext, CL_MEM_READ_ONLY |
         CL_MEM_COPY_HOST_PTR, sizeof(int) , &HostVectorB, NULL);
         
         // Allocate output memory on GPU
         cl_mem GPUOutputVector0 = clCreateBuffer(GPUContext, CL_MEM_WRITE_ONLY,
         sizeof(int) * SIZE, NULL, NULL);
         // Allocate output memory on GPU
         cl_mem GPUOutputVector1 = clCreateBuffer(GPUContext, CL_MEM_WRITE_ONLY,
         sizeof(int) * SIZE, NULL, NULL);
         
         // In the next step we associate the GPU memory with the Kernel arguments
         clSetKernelArg(OpenCLVectorAdd, 0, sizeof(cl_mem), (void*)&GPUOutputVector0);
         clSetKernelArg(OpenCLVectorAdd, 1, sizeof(cl_mem), (void*)&GPUOutputVector1);
         clSetKernelArg(OpenCLVectorAdd, 2, sizeof(cl_mem), (void*)&GPUVectorB);
         clSetKernelArg(OpenCLVectorAdd, 3, sizeof(cl_mem), (void*)&GPUVector1);
         clSetKernelArg(OpenCLVectorAdd, 4, sizeof(cl_mem), (void*)&GPUVector2);
         
         // Launch the Kernel on the GPU
         // This kernel only uses global data
         size_t WorkSize[1] = {SIZE}; // one dimensional Range
         clEnqueueNDRangeKernel(cqCommandQueue, OpenCLVectorAdd, 1, NULL,
         WorkSize, NULL, 0, NULL, NULL);
         
         // Copy the output in GPU memory back to CPU memory
         clEnqueueReadBuffer(cqCommandQueue, GPUOutputVector0, CL_TRUE, 0,
         (sizeof(int) * SIZE), HostOutputVector0, 0, NULL, NULL);
         // Copy the output in GPU memory back to CPU memory
         clEnqueueReadBuffer(cqCommandQueue, GPUOutputVector1, CL_TRUE, 0,
         (sizeof(int) * SIZE), HostOutputVector1, 0, NULL, NULL);
         
         int i;
         for( i=0 ; i < SIZE; i++)
              printf("[%d + %d = %d , %d]\n",HostVector1[i], HostVector2[i], HostOutputVector1[i],HostOutputVector0[i]);
         
         printf("\n");
         
         bKeekCalc = KeepCalc(HostOutputVector1,SIZE);
         
         if (bKeekCalc) 
         {
            copy_array(HostOutputVector1,HostVector1,SIZE);
            copy_array(HostOutputVector0,HostVector2,SIZE);
         }
         
         // Cleanup
         clReleaseMemObject(GPUVector1);
         clReleaseMemObject(GPUVector2);
         clReleaseMemObject(GPUVectorB);         
         clReleaseMemObject(GPUOutputVector0);
         clReleaseMemObject(GPUOutputVector1);
         
     } while (bKeekCalc);
    
    // Cleanup
    clReleaseContext(GPUContext);
    clReleaseProgram(OpenCLProgram);
    clReleaseKernel(OpenCLVectorAdd);
    clReleaseCommandQueue(cqCommandQueue);

 return 0;
}
