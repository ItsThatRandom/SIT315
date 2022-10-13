#include <stdio.h>
#include <stdlib.h>
#include <CL/cl.h>

#define PRINT 1

int SZ = 8;
int *v;

// Creation of the required buffers/context variables.
// Variable for the memory buffer to store elements.
cl_mem bufV;
// The compute device's ID.
cl_device_id device_id;
// The context / environment.
cl_context context;
// The host program.
cl_program program;
// The kernel program to be executed.
cl_kernel kernel;
// The command queue to store the sent commands.
cl_command_queue queue;
cl_event event = NULL;

int err;

// Creates a new device ID, determines platform and then first if its a GPU, if not then a CPU, or neither and throws error.
cl_device_id create_device();
// Creates the resources required (Context, command queue, program, kernel).
void setup_openCL_device_context_queue_kernel(char *filename, char *kernelname);
// Creates a cl_program for the given context and device by reading in from a source file.
cl_program build_program(cl_context ctx, cl_device_id dev, const char *filename);
// Allocates data to the kernels local memory to work on.
void setup_kernel_memory();
// Copy kernel arguements and set them.
void copy_kernel_args();
// Free up the memory no longer being used.
void free_memory();

void init(int *&A, int size);
void print(int *A, int size);

int main(int argc, char **argv)
{
    if (argc > 1)
        SZ = atoi(argv[1]);

    init(v, SZ);


    // Create kernel work groups as single dimension arrays.
    size_t global[1] = {(size_t)SZ};

    //initial vector
    print(v, SZ);

    setup_openCL_device_context_queue_kernel((char *)"./vector_ops.cl", (char *)"square_magnitude");

    setup_kernel_memory();
    copy_kernel_args();

    // Used to add a command to a devices command queue to execute a kernel (Command Queue, Kernel, Work Dimension, Global Work Offset, 
    // Global Work Size, Local Work Size, Event Wait List, Number of Events, Event).
    clEnqueueNDRangeKernel(queue, kernel, 1, NULL, global, NULL, 0, NULL, &event);
    clWaitForEvents(1, &event);

    // Used to add a command to read from the devices buffer into host memory (Command Queue, Buffer, Blocking/Non-Blocking, Offset, Byte Size, 
    // Host Buffer Memory Pointer, Event Wait List, Number of Events, Event).
    clEnqueueReadBuffer(queue, bufV, CL_TRUE, 0, SZ * sizeof(int), &v[0], 0, NULL, NULL);

    //result vector
    print(v, SZ);

    //frees memory for device, kernel, queue, etc.
    //you will need to modify this to free your own buffers
    free_memory();
}

void init(int *&A, int size)
{
    A = (int *)malloc(sizeof(int) * size);

    for (long i = 0; i < size; i++)
    {
        A[i] = rand() % 100; // any number less than 100
    }
}

void print(int *A, int size)
{
    if (PRINT == 0)
    {
        return;
    }

    if (PRINT == 1 && size > 15)
    {
        for (long i = 0; i < 5; i++)
        {                        //rows
            printf("%d ", A[i]); // print the cell value
        }
        printf(" ..... ");
        for (long i = size - 5; i < size; i++)
        {                        //rows
            printf("%d ", A[i]); // print the cell value
        }
    }
    else
    {
        for (long i = 0; i < size; i++)
        {                        //rows
            printf("%d ", A[i]); // print the cell value
        }
    }
    printf("\n----------------------------\n");
}

void free_memory()
{
    //free the buffers
    clReleaseMemObject(bufV);

    //free opencl objects
    clReleaseKernel(kernel);
    clReleaseCommandQueue(queue);
    clReleaseProgram(program);
    clReleaseContext(context);

    free(v);
}


void copy_kernel_args()
{
    // Function sets the arguements and queries for the specified kernel (Kernel, Arg_Index, Arg_Size, Arg_Value)
    clSetKernelArg(kernel, 0, sizeof(int), (void *)&SZ);
    clSetKernelArg(kernel, 1, sizeof(cl_mem), (void *)&bufV);

    if (err < 0)
    {
        perror("Couldn't create a kernel argument");
        printf("error = %d", err);
        exit(1);
    }
}

void setup_kernel_memory()
{ 
    // Function creates the buffer object storing elements to be accessed by the device executing the kernal (Context, Flags, Size, Host Pointer, Error Code)
    // Memory flag value examples: read/write, read OR write, host no access, host read OR write, use/allocate/copy host pointer.
    bufV = clCreateBuffer(context, CL_MEM_READ_WRITE, SZ * sizeof(int), NULL, NULL);

    // Copy matrices to the GPU
    clEnqueueWriteBuffer(queue, bufV, CL_TRUE, 0, SZ * sizeof(int), &v[0], 0, NULL, NULL);
}

void setup_openCL_device_context_queue_kernel(char *filename, char *kernelname)
{
    device_id = create_device();
    cl_int err;

    // Function creates the context and links the devices associated with it (Properties, Number Of Devices, Devices, CL_CALLBACK(...), User Data, Error Code).
    context = clCreateContext(NULL, 1, &device_id, NULL, NULL, &err);
    if (err < 0)
    {
        perror("Couldn't create a context");
        exit(1);
    }

    program = build_program(context, device_id, filename);

    // Function creates a command queue with the given paramaters (Context, Device, Command Queue Properties, Error Code).
    queue = clCreateCommandQueueWithProperties(context, device_id, 0, &err);
    if (err < 0)
    {
        perror("Couldn't create a command queue");
        exit(1);
    };


    kernel = clCreateKernel(program, kernelname, &err);
    if (err < 0)
    {
        perror("Couldn't create a kernel");
        printf("error =%d", err);
        exit(1);
    };
}

cl_program build_program(cl_context ctx, cl_device_id dev, const char *filename)
{

    cl_program program;
    FILE *program_handle;
    char *program_buffer, *program_log;
    size_t program_size, log_size;

    /* Read program file and place content into buffer */
    program_handle = fopen(filename, "r");
    if (program_handle == NULL)
    {
        perror("Couldn't find the program file");
        exit(1);
    }
    fseek(program_handle, 0, SEEK_END);
    program_size = ftell(program_handle);
    rewind(program_handle);
    program_buffer = (char *)malloc(program_size + 1);
    program_buffer[program_size] = '\0';
    fread(program_buffer, sizeof(char), program_size, program_handle);
    fclose(program_handle);

    // Function creates a cl_program using source code provided in paramater (Context, Count, Strings, Lengths, Error Code).
    program = clCreateProgramWithSource(ctx, 1, (const char **)&program_buffer, &program_size, &err);
    if (err < 0)
    {
        perror("Couldn't create the program");
        exit(1);
    }
    free(program_buffer);

    /* Build program 

   The fourth parameter accepts options that configure the compilation. 
   These are similar to the flags used by gcc. For example, you can 
   define a macro with the option -DMACRO=VALUE and turn off optimization 
   with -cl-opt-disable.
   */
    err = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
    if (err < 0)
    {

        /* Find size of log and print to std output */
        clGetProgramBuildInfo(program, dev, CL_PROGRAM_BUILD_LOG,
                              0, NULL, &log_size);
        program_log = (char *)malloc(log_size + 1);
        program_log[log_size] = '\0';
        clGetProgramBuildInfo(program, dev, CL_PROGRAM_BUILD_LOG,
                              log_size + 1, program_log, NULL);
        printf("%s\n", program_log);
        free(program_log);
        exit(1);
    }

    return program;
}

cl_device_id create_device() {

   cl_platform_id platform;
   cl_device_id dev;
   int err;

   /* Identify a platform */
   err = clGetPlatformIDs(1, &platform, NULL);
   if(err < 0) {
      perror("Couldn't identify a platform");
      exit(1);
   } 

   // Access a device
   // GPU
   err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &dev, NULL);
   if(err == CL_DEVICE_NOT_FOUND) {
      // CPU
      printf("GPU not found\n");
      err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_CPU, 1, &dev, NULL);
   }
   if(err < 0) {
      perror("Couldn't access any devices");
      exit(1);   
   }

   return dev;
}

// --------------------------------------------------------------- //
// ----------------------- vector_ops.cl ------------------------- //
// --------------------------------------------------------------- //
// // Addition function/kernal program that will distributed 
// // and executed on all compute devices.
// __kernel void square_magnitude(const int size,
//                       __global int* v) {
    
//     // Thread identifiers
//     const int globalIndex = get_global_id(0);   
 
//     //uncomment to see the index each PE works on
//     //printf("Kernel process index :(%d)\n ", globalIndex);

//     v[globalIndex] = v[globalIndex] * v[globalIndex];
// }