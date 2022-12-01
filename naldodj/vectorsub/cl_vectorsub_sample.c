//************************************************************
// Demo OpenCL application to compute a simple vector Subition
// computation between 2 arrays on the GPU
// ************************************************************
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <CL/cl.h>

// Some interesting data for the vectors
// **** Keep the zero in left ****
#define DATA_SIZE 42
unsigned int InitialData1[DATA_SIZE] = {0,8,4,3,5,7,0,8,3,9,2,4,5,6,7,8,9,0,1,2,3,0,8,4,3,5,7,0,8,3,9,2,4,5,6,7,8,9,0,1,2,3};
unsigned int InitialData2[DATA_SIZE] = {0,2,5,7,8,5,9,9,4,3,9,7,5,6,8,8,9,9,9,8,7,0,2,5,7,8,5,9,9,4,3,9,7,5,6,8,8,9,9,9,8,7};

// Number of elements in the vectors to be Subed
#define SIZE 144
/* if SIZE == 144 then
084357083924567890123084357083924567890123084357083924567890123084357083924567890123084357083924567890123084357083924567890123084357083924567890 
025785994397568899987025785994397568899987025785994397568899987025785994397568899987025785994397568899987025785994397568899987025785994397568899 (-)
------------------------------------------------------------------------------------------------------------------------------------------------
058571089526998990136058571089526998990136058571089526998990136058571089526998990136058571089526998990136058571089526998990136058571089526998991 (=)
*/

// Max Number of elements in the vectors to be Subed
#define MAX_SIZE 16384 // ?

// OpenCL source code
const char* OpenCLSource = {
    "__kernel void VectorSub(__global unsigned int* restrict iVRet1"
    "                       ,__global unsigned int* restrict iVRet2"
    "                       ,constant unsigned int* restrict iBase"
    "                       ,constant unsigned int* restrict iVGet1"
    "                       ,constant unsigned int* restrict iVGet2)"
    "{"
    "   unsigned int n = get_global_id(0);"
    "   int ivG1 = (int)iVGet1[n];"
    "   int ivG2 = (int)iVGet2[n];"
    "   int s = (ivG1-ivG2);"
    "   int b = (int)iBase[0];"
    "   bool bChange = (s<0);"
    "   int v=s;"
    "   unsigned int v1=0;"
    "   if (bChange) {"
    "       v+=b;"
    "       v1=1;"
    "   }"
    "   iVRet2[n-1] = v1;"
    "   iVRet1[n] = (unsigned int)v;"
    "}"
};

void copy_array(unsigned int source_arr[], unsigned int target_arr[], unsigned long long int size)
{

    unsigned int *source_ptrs = source_arr;
    unsigned int *target_ptrs = target_arr;

    unsigned int *source_ptre = &source_arr[size - 1];
    unsigned int *target_ptre = &target_arr[size - 1];

    while(source_ptrs <= source_ptre)
    {

        *target_ptrs = *source_ptrs;
        *target_ptre = *source_ptre;

        source_ptrs++;
        target_ptrs++;

        source_ptre--;
        target_ptre--;

    }

}

bool KeepCalc(unsigned int arr[], unsigned long long int size)
{

    if (size<4)
    {

        unsigned int *p1 = arr;
        unsigned int *p2 = (arr+(size-1));

        while (p1 < p2) {
            if ( *p1++ || *p2-- )
            {
                return(true);
            }
        }
        
        return(false);

    } else {

        unsigned int *p1 = arr;
        unsigned int *p2 = (arr+((size-1)/2));

        unsigned int *p3 = (p2+1);
        unsigned int *p4 = (arr+(size-1));

        while (p1 < p2) {
            if ( *p1++ || *p2-- )
            {
                return(true);
            }
            if (p3 < p4) {
                if ( *p3++ || *p4-- )
                {
                    return(true);
                }
            }
        }

        return(false);

    }

}

unsigned int * VectorSub(unsigned int HostVector1[],unsigned int HostVector2[],unsigned int HostVectorB, unsigned long long int nSize,cl_context GPUContext,cl_kernel OpenCLVectorSub,cl_command_queue cqCommandQueue,unsigned int HostOutputVector0[])
{
    unsigned int HostOutputVector1[nSize];

    // Initialize with some interesting repeating data
    unsigned int c;
    for(c = 0; c < nSize; c++)
    {
      HostOutputVector0[c] = 0;
      HostOutputVector1[c] = 0;
    }    
    
    bool bKeepCalc = false;
    do
    {

        // Allocate GPU memory for source vectors AND initialize from CPU memory
        cl_mem GPUVector1 = clCreateBuffer(GPUContext, CL_MEM_READ_ONLY |
        CL_MEM_COPY_HOST_PTR, sizeof(unsigned int) * nSize, HostVector1, NULL);
        cl_mem GPUVector2 = clCreateBuffer(GPUContext, CL_MEM_READ_ONLY |
        CL_MEM_COPY_HOST_PTR, sizeof(unsigned int) * nSize, HostVector2, NULL);
        cl_mem GPUVectorB = clCreateBuffer(GPUContext, CL_MEM_READ_ONLY |
        CL_MEM_COPY_HOST_PTR, sizeof(unsigned int) , &HostVectorB, NULL);

        // Allocate output memory on GPU
        cl_mem GPUOutputVector0 = clCreateBuffer(GPUContext, CL_MEM_WRITE_ONLY|
        CL_MEM_COPY_HOST_PTR,
        sizeof(unsigned int) * nSize, HostOutputVector0, NULL);
        // Allocate output memory on GPU
        cl_mem GPUOutputVector1 = clCreateBuffer(GPUContext, CL_MEM_WRITE_ONLY|
        CL_MEM_COPY_HOST_PTR,
        sizeof(unsigned int) * nSize, HostOutputVector1, NULL);

        // In the next step we associate the GPU memory with the Kernel arguments
        clSetKernelArg(OpenCLVectorSub, 0, sizeof(cl_mem), (void*)&GPUOutputVector0);
        clSetKernelArg(OpenCLVectorSub, 1, sizeof(cl_mem), (void*)&GPUOutputVector1);
        clSetKernelArg(OpenCLVectorSub, 2, sizeof(cl_mem), (void*)&GPUVectorB);
        clSetKernelArg(OpenCLVectorSub, 3, sizeof(cl_mem), (void*)&GPUVector1);
        clSetKernelArg(OpenCLVectorSub, 4, sizeof(cl_mem), (void*)&GPUVector2);

        // Launch the Kernel on the GPU
        // This kernel only uses global data
        size_t WorkSize[1] = {nSize}; // one dimensional Range
        clEnqueueNDRangeKernel(cqCommandQueue, OpenCLVectorSub, 1, NULL,
        WorkSize, NULL, 0, NULL, NULL);

        // Copy the output in GPU memory back to CPU memory
        clEnqueueReadBuffer(cqCommandQueue, GPUOutputVector0, CL_TRUE, 0,
        (sizeof(unsigned int) * nSize), HostOutputVector0, 0, NULL, NULL);
        // Copy the output in GPU memory back to CPU memory
        clEnqueueReadBuffer(cqCommandQueue, GPUOutputVector1, CL_TRUE, 0,
        (sizeof(unsigned int) * nSize), HostOutputVector1, 0, NULL, NULL);

        bKeepCalc = KeepCalc(HostOutputVector1,nSize);

        if (bKeepCalc)
        {
            copy_array(HostOutputVector0,HostVector1,nSize);
            copy_array(HostOutputVector1,HostVector2,nSize);
        }

        // Cleanup
        clReleaseMemObject(GPUVector1);
        clReleaseMemObject(GPUVector2);
        clReleaseMemObject(GPUVectorB);
        clReleaseMemObject(GPUOutputVector0);
        clReleaseMemObject(GPUOutputVector1);

    } while (bKeepCalc);

    return((unsigned int*)HostOutputVector0);    
    
}

// Main function
// ************************************************************
int main(int argc, char **argv)
{

    unsigned long long int nSize=(unsigned long long int)SIZE;
    unsigned long long int nMaxSize=(unsigned long long int)MAX_SIZE;

    switch(argc)
    {
        case 2:          /* One parameter -- use input file & stdout. */
        nSize = (unsigned long long int)atoi( argv[1] );
        if (!nSize) {
            puts("invalid value.\n");
            exit( 0 );
        }
        if (nSize>nMaxSize) {
          printf("invalid value [%u]. Using MAX_SIZE [%u]\n",nSize,nMaxSize);
          nSize=nMaxSize;
        }
        break;
    }

    // Two integer source vectors in Host memory
    unsigned int HostVector1[nSize], HostVector2[nSize], HostVectorB=10;

    //Output Vector
    unsigned int HostOutputVector0[nSize];

    // Initialize with some interesting repeating data
    unsigned int c;
    for(c = 0; c < nSize; c++)
    {
      HostVector1[c] = InitialData1[c%DATA_SIZE];
      HostVector2[c] = InitialData2[c%DATA_SIZE];
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
    cl_program OpenCLProgram = clCreateProgramWithSource(GPUContext, 1, (const char **) &OpenCLSource, NULL, NULL);

    // Build the program (OpenCL JIT compilation)
    clBuildProgram(OpenCLProgram, 0, NULL, NULL, NULL, NULL);

    // Create a handle to the compiled OpenCL function (Kernel)
    cl_kernel OpenCLVectorSub = clCreateKernel(OpenCLProgram, "VectorSub", NULL);

    unsigned int i;

    for( i=0 ; i < nSize; i++)
        printf("%u",HostVector1[i]);
    printf("\n");

    for( i=0 ; i < nSize; i++)
        printf("%u",HostVector2[i]);
    printf(" (-)\n");

    for( i=0 ; i < ((nSize>144)?144:nSize); i++)
        printf("-");
    printf("\n");

    *HostOutputVector0=*VectorSub(HostVector1,HostVector2,HostVectorB,nSize,GPUContext,OpenCLVectorSub,cqCommandQueue,HostOutputVector0);

    for( i=0 ; i < nSize; i++)
      printf("%u",HostOutputVector0[i]);
    printf(" (=)\n");

    printf("\n");

    // Cleanup
    clReleaseContext(GPUContext);
    clReleaseProgram(OpenCLProgram);
    clReleaseKernel(OpenCLVectorSub);
    clReleaseCommandQueue(cqCommandQueue);

 return 0;
}
