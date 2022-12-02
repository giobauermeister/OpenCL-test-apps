//************************************************************
// Demo OpenCL application to compute a simple vector Subition
// computation between 2 arrays on the GPU
// ************************************************************
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <limits.h>
#include <inttypes.h>

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
#define MAX_SIZE 9999 //ULONG_MAX // ?

// OpenCL source code
const char* OpenCLSource = {
    "__kernel void VectorSub(__global char* restrict iVRet1"
    "                       ,__global char* restrict iVRet2"
    "                       ,constant unsigned int* restrict iBase"
    "                       ,constant char* restrict iVGet1"
    "                       ,constant char* restrict iVGet2)"
    "{"
    "   unsigned int n = get_global_id(0);"
    "   int ivG1 = (int)(iVGet1[n]-'0');"
    "   int ivG2 = (int)(iVGet2[n]-'0');"
    "   int s = (ivG1-ivG2);"
    "   int b = iBase[0];"
    "   bool bChange = (s<0);"
    "   int v=s;"
    "   unsigned int v1=0;"
    "   if (bChange) {"
    "       v+=b;"
    "       v1=1;"
    "   }"
    "   iVRet2[n-1] = v1+'0';"
    "   iVRet1[n] = ((unsigned int)v)+'0';"
    "}"
};

static char cNumber(const long long unsigned int iNumber){
    char cNumber;
    static const char * st__sNumber="0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    static const long long unsigned int st_iNumber=strlen(st__sNumber);
    cNumber=st__sNumber[(iNumber<=st_iNumber?iNumber:0)];
    return(cNumber);
}

static unsigned long long int iNumber(const char * cNumber){

    const char cN=*(cNumber);

    long long int iNumber;

    if (isdigit(cN))
    {
        iNumber=(cN-'0');
    }
    else
    {

        int j=(-1);

        if (isalpha(cN))
        {

            long long int i;

            static const char * st__cNumber[62]={"0","1","2","3","4","5","6","7","8","9"
                                            ,"A","B","C","D","E","F","G","H","I","J"
                                            ,"K","L","M","N","O","P","Q","R","S","T"
                                            ,"U","V","W","X","Y","Z","a","b","c","d"
                                            ,"e","f","g","h","i","j","k","l","m","n"
                                            ,"o","p","q","r","s","t","u","v","w","x"
                                            ,"y","z"
                                };

            for (i=0;(i<sizeof(st__cNumber));i++)
            {
                if (strncmp(cNumber,st__cNumber[i],1)==0)
                {
                    j=i;
                    break;
                }
            }

        }

        iNumber=( j >= 0 ? j : 0 );

    }
    return(iNumber);
}

void copy_array(char * source_arr, char * target_arr)
{

    unsigned long long int source_ptrs = 0;
    unsigned long long int target_ptrs = 0;

    unsigned long long int source_ptre = strlen(source_arr);
    unsigned long long int target_ptre = strlen(target_arr);

    while(source_ptrs <= source_ptre)
    {
        target_arr[target_ptrs++] = source_arr[source_ptrs++];
        target_arr[target_ptre--] = source_arr[source_ptre--];
    }

}

bool KeepCalc(char * arr)
{
    return(strstr(arr,"1" )!=NULL);
}

char * VectorSub(char * HostVector1,char * HostVector2,unsigned int HostVectorB, unsigned long long int nSize,cl_context GPUContext,cl_kernel OpenCLVectorSub,cl_command_queue cqCommandQueue,char * HostOutputVector0)
{
    
    char * HostOutputVector1=(char*)malloc(nSize*sizeof(char*));

    // Initialize with some interesting repeating data
    int c;
    for(c = 0; c < nSize; c++)
    {
      HostOutputVector0[c] = cNumber(0);
      HostOutputVector1[c] = cNumber(0);
    }

    HostOutputVector0[c]='\0';
    HostOutputVector1[c]='\0';
    
    bool bKeepCalc = false;
    do
    {

        // Allocate GPU memory for source vectors AND initialize from CPU memory
        cl_mem GPUVector1 = clCreateBuffer(GPUContext, CL_MEM_READ_ONLY |
        CL_MEM_COPY_HOST_PTR, sizeof(char *) * nSize, HostVector1, NULL);
        cl_mem GPUVector2 = clCreateBuffer(GPUContext, CL_MEM_READ_ONLY |
        CL_MEM_COPY_HOST_PTR, sizeof(char *) * nSize, HostVector2, NULL);
        cl_mem GPUVectorB = clCreateBuffer(GPUContext, CL_MEM_READ_ONLY |
        CL_MEM_COPY_HOST_PTR, sizeof(unsigned int*) , &HostVectorB, NULL);

        // Allocate output memory on GPU
        cl_mem GPUOutputVector0 = clCreateBuffer(GPUContext, CL_MEM_WRITE_ONLY|
        CL_MEM_COPY_HOST_PTR,
        sizeof(char *) * nSize, HostOutputVector0, NULL);
        // Allocate output memory on GPU
        cl_mem GPUOutputVector1 = clCreateBuffer(GPUContext, CL_MEM_WRITE_ONLY|
        CL_MEM_COPY_HOST_PTR,
        sizeof(char *) * nSize, HostOutputVector1, NULL);

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
        (sizeof(char *) * nSize), HostOutputVector0, 0, NULL, NULL);
        // Copy the output in GPU memory back to CPU memory
        clEnqueueReadBuffer(cqCommandQueue, GPUOutputVector1, CL_TRUE, 0,
        (sizeof(char *) * nSize), HostOutputVector1, 0, NULL, NULL);

        bKeepCalc = KeepCalc(HostOutputVector1);

        if (bKeepCalc)
        {
            copy_array(HostOutputVector0,HostVector1);
            copy_array(HostOutputVector1,HostVector2);
        }

        // Cleanup
        clReleaseMemObject(GPUVector1);
        clReleaseMemObject(GPUVector2);
        clReleaseMemObject(GPUVectorB);
        clReleaseMemObject(GPUOutputVector0);
        clReleaseMemObject(GPUOutputVector1);

    } while (bKeepCalc);    
    
    return((char *)HostOutputVector0);
    
}

// Main function
// ************************************************************
int main(int argc, char **argv)
{

    unsigned long long int nSize=(SIZE);
    unsigned long long int nMaxSize=(MAX_SIZE);

    switch(argc)
    {
        case 2:          /* One parameter -- use input file & stdout. */
        char *stopstring;
        nSize = (unsigned long long int)strtoull( argv[1] , &stopstring , 10 );
        if (!nSize) {
            puts("invalid value.\n");
            exit( 0 );
        }
        if (nSize>nMaxSize) {
          printf("invalid value [%llu]. Using MAX_SIZE [%llu]\n",nSize,nMaxSize);
          nSize=nMaxSize;
        }
        break;
    }

    // Two integer source vectors in Host memory
    char * HostVector1=(char*)malloc(nSize*sizeof(char*));
    char * HostVector2=(char*)malloc(nSize*sizeof(char*));

    unsigned int HostVectorB=10;

    //Output Vector
    char * HostOutputVector0=(char*)malloc(nSize*sizeof(char*));

    // Initialize with some interesting repeating data
    int c;
    for(c = 0; c < nSize; c++)
    {
      HostVector1[c] = cNumber(InitialData1[c%DATA_SIZE]);
      HostVector2[c] = cNumber(InitialData2[c%DATA_SIZE]);
    }

    HostVector1[c]='\0';
    HostVector2[c]='\0';

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

    printf("%s\n",HostVector1);
    printf("");
    
    printf("%s (-)\n",HostVector2);
    
    int i;
    for( i=0 ; i < ((nSize>144)?144:nSize); i++)
        printf("-");
    printf("\n");

    *HostOutputVector0=*VectorSub(HostVector1,HostVector2,HostVectorB,nSize,GPUContext,OpenCLVectorSub,cqCommandQueue,HostOutputVector0);

    printf("%s (=)\n",HostOutputVector0);

    printf("\n");

    // Cleanup
    clReleaseContext(GPUContext);
    clReleaseProgram(OpenCLProgram);
    clReleaseKernel(OpenCLVectorSub);
    clReleaseCommandQueue(cqCommandQueue);

/*
    free(HostVector1);
    free(HostVector2);
    free(HostOutputVector0);
    free(HostOutputVector1);
*/

 return 0;
}
