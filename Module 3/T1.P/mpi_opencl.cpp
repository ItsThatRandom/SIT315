// Code based on reference material provided through class Teams channel.

#include <stdio.h>
#include <stdlib.h>
#include <CL/cl.h>
#include <time.h>
#include <thread>
#include <chrono>
#include <mpi.h>

using namespace std::chrono;


int SZ = 4;
int **v, **v2, **v3;

// Creation of the required buffers/context variables.
// Variable for the memory buffer to store elements.
cl_mem bufV, bufV2, bufV3;
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

const int TS = 4;
size_t local[2] = { (size_t)TS, (size_t)TS };
size_t global[2] = { (size_t) SZ, (size_t)SZ }; 

// Creates a new device ID, determines platform and then first if its a GPU, if not then a CPU, or neither and throws error.
cl_device_id create_device();
// Creates the resources required (Context, command queue, program, kernel).
void setup_openCL_device_context_queue_kernel(char *filename, char *kernelname);
// Creates a cl_program for the given context and device by reading in from a source file.
cl_program build_program(cl_context ctx, cl_device_id dev, const char *filename);
// Allocates data to the kernels local memory to work on.
void setup_kernel_memory(int process_rows);
// Copy kernel arguements and set them.
void copy_kernel_args(int process_rows);
// Free up the memory no longer being used.
void free_memory();

void init(int **&v, int rows, int cols, bool initial);
void add(int **&v, int **&v2, int **&v3, int rows, int cols);
void print(int **v, int rows, int cols);

int main(int argc, char **argv)
{
    if (argc > 1)
        SZ = atoi(argv[1]);

    int numtasks, rank; 

    // Initialize the MPI environment
    MPI_Init(&argc,&argv);

    // Get the number of tasks/process
    MPI_Comm_size(MPI_COMM_WORLD, &numtasks);

    // Get the rank
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    // Head
    if (rank == 0) {
        init(v, SZ, SZ, true);
        init(v2, SZ, SZ, true);
        init(v3, SZ, SZ, false);

        // print(v, SZ, SZ);
        // print(v2, SZ, SZ);

        

        int process_rows = SZ / numtasks;
        int bcast_elements = (SZ * SZ);
        int local_elements = (SZ * SZ) / numtasks;

        // printf("numtasks: %d.\nProcess rows: %d.\nbcast elements: %d\nLocal elements: %d\n", numtasks, process_rows, bcast_elements, local_elements);

        auto start = high_resolution_clock::now();

        MPI_Scatter(&v[0][0], local_elements, MPI_INT, &v[0][0], 0, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Bcast(&v2[0][0], bcast_elements, MPI_INT, 0, MPI_COMM_WORLD);

        // Open CL
        local[0] = process_rows;
        local[1] = SZ;
        global[0] = process_rows;
        global[1] = SZ;
      
        // printf("global[0]: %lu.\nglobal[1]: %lu.\nlocal[0]: %lu\nlocal[1]: %lu\n", global[0], global[1], local[0], local[1]);

        setup_openCL_device_context_queue_kernel((char *)"./mpi_opencl.cl", (char *)"multiply_matrices");
        setup_kernel_memory(process_rows);
        copy_kernel_args(process_rows);

        // Ensure enqueue successful or print error code.
        cl_int res = clEnqueueNDRangeKernel(queue, kernel, 2, NULL, global, local, 0, NULL, &event);
        if (res != CL_SUCCESS) {
            printf("Failed to enqueue. CODE: %d\n", res);
        }

        clWaitForEvents(1, &event);
        clEnqueueReadBuffer(queue, bufV3, CL_TRUE, 0, process_rows * SZ * sizeof(int), &v3[0][0], 0, NULL, NULL);
        // End Open CL

        MPI_Gather(MPI_IN_PLACE, local_elements, MPI_INT, &v3[0][0], local_elements, MPI_INT, 0, MPI_COMM_WORLD);

        // Store the time (stop time) in a variable
        auto stop = high_resolution_clock::now();

        // Calculates the time difference (duration of execution)
        auto duration = duration_cast<microseconds>(stop - start);

        print(v3, SZ, SZ);
        printf("Time taken by function: %ld microseconds.\n", duration.count());

        free_memory();
    }

    // Node
    else {

        int process_rows = SZ / numtasks;
        int bcast_elements = (SZ * SZ);
        int local_elements = (SZ * SZ) / numtasks;

        init(v, process_rows, SZ, true);
        init(v2, SZ, SZ, true);
        init(v3, process_rows, SZ, false);

        // printf("numtasks: %d.\nProcess rows: %d.\nbcast elements: %d\nLocal elements: %d\n", numtasks, process_rows, bcast_elements, local_elements);

        MPI_Scatter(NULL, local_elements, MPI_INT, &v[0][0], local_elements, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Bcast(&v2[0][0], bcast_elements, MPI_INT, 0, MPI_COMM_WORLD);

        // Open CL
        local[0] = process_rows;
        local[1] = SZ;
        global[0] = process_rows;
        global[1] = SZ;
    
        setup_openCL_device_context_queue_kernel((char *)"./mpi_opencl.cl", (char *)"multiply_matrices");
        setup_kernel_memory(process_rows);
        copy_kernel_args(process_rows);

        // Ensure enqueue successful or print error code.
        cl_int res = clEnqueueNDRangeKernel(queue, kernel, 2, NULL, global, local, 0, NULL, &event);
        if (res != CL_SUCCESS) {
            printf("Failed to enqueue. CODE: %d\n", res);
        }

        clWaitForEvents(1, &event);
        clEnqueueReadBuffer(queue, bufV3, CL_TRUE, 0, process_rows * SZ * sizeof(int), &v3[0][0], 0, NULL, NULL);
        // End Open CL

        MPI_Gather(&v3[0][0], local_elements, MPI_INT, NULL, local_elements, MPI_INT, 0, MPI_COMM_WORLD);
        free_memory();
    }
    MPI_Finalize();
}

void init(int **&v, int rows, int cols, bool initial)
{
    v = (int **)malloc(sizeof(int*) * rows * cols);
    int* temp = (int *)malloc(sizeof(int) * rows * cols);

    for (int i = 0; i < SZ; i++) {
        v[i] = &temp[i * cols];
    }

    if (!initial) return;

    for (long i = 0; i < rows; i++) {
        for (long j = 0; j < cols; j++) {
            v[i][j] = rand() % 10; 
        }
    }
}

void print(int **v, int rows, int cols)
{
    for(long i = 0 ; i < rows; i++) { //rows
        for(long j = 0 ; j < cols; j++) {  //cols
            printf("%d ",  v[i][j]); // print the cell value
        }
        printf("\n"); //at the end of the row, print a new line
    }
    printf("----------------------------\n");
}

void free_memory()
{
    //free the buffers
    clReleaseMemObject(bufV);
    clReleaseMemObject(bufV2);
    clReleaseMemObject(bufV3);

    //free opencl objects
    clReleaseKernel(kernel);
    clReleaseCommandQueue(queue);
    clReleaseProgram(program);
    clReleaseContext(context);

    free(v);
    free(v2);
    free(v3);
}


void copy_kernel_args(int process_rows)
{
    // Function sets the arguements and queries for the specified kernel (Kernel, Arg_Index, Arg_Size, Arg_Value)
    clSetKernelArg(kernel, 0, sizeof(int), (void *)&process_rows);
    clSetKernelArg(kernel, 1, sizeof(int), (void *)&SZ);
    clSetKernelArg(kernel, 2, sizeof(int), (void *)&SZ);
    clSetKernelArg(kernel, 3, sizeof(cl_mem), (void *)&bufV);
    clSetKernelArg(kernel, 4, sizeof(cl_mem), (void *)&bufV2);
    clSetKernelArg(kernel, 5, sizeof(cl_mem), (void *)&bufV3);

    if (err < 0)
    {
        perror("Couldn't create a kernel argument");
        printf("error = %d", err);
        exit(1);
    }
}

void setup_kernel_memory(int process_rows)
{ 
    // Function creates the buffer object storing elements to be accessed by the device executing the kernal (Context, Flags, Size, Host Pointer, Error Code)
    // Memory flag value examples: read/write, read OR write, host no access, host read OR write, use/allocate/copy host pointer.
    bufV = clCreateBuffer(context, CL_MEM_READ_ONLY, process_rows * SZ * sizeof(int), NULL, NULL);
    bufV2 = clCreateBuffer(context, CL_MEM_READ_ONLY, SZ * SZ * sizeof(int), NULL, NULL);
    bufV3 = clCreateBuffer(context, CL_MEM_READ_WRITE, process_rows * SZ * sizeof(int), NULL, NULL);

    // Copy matrices to the GPU
    clEnqueueWriteBuffer(queue, bufV, CL_TRUE, 0, process_rows * SZ * sizeof(int), &v[0][0], 0, NULL, NULL);
    clEnqueueWriteBuffer(queue, bufV2, CL_TRUE, 0, SZ * SZ * sizeof(int), &v2[0][0], 0, NULL, NULL);
    clEnqueueWriteBuffer(queue, bufV3, CL_TRUE, 0, process_rows * SZ * sizeof(int), &v3[0][0], 0, NULL, NULL);
}

void setup_openCL_device_context_queue_kernel(char *filename, char *kernelname)
{
    device_id = create_device();
    cl_int err;

    // // Check device max work group size.
    // size_t size;
    // clGetDeviceInfo(device_id, CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(size), &size, NULL);
    // printf("\tCL_DEVICE_MAX_WORK_GROUP_SIZE: %lu\n", size);

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

// ------------------------------------------------- KERNAL ---------------------------------------------------------------//
// ---------------------------------------- File Name: mpi_opencl.cl ------------------------------------------------------//
// ------------------------------------------------------------------------------------------------------------------------//

// __kernel void multiply_matrices(const int M, const int N, const int K,
//                       const __global int* v,
//                       const __global int* v2,
//                       __global int* v3) {
    
//     // Thread identifierss
//     const int globalRow = get_global_id(0); // Row ID of C (0..M)
//     const int globalCol = get_global_id(1); // Col ID of C (0..N)
//     //printf("M: %d, N: %d, K: %d\n", M, N, K);
    
//     // Compute a single element (loop over K)
    
    
//     int localSum = 0;
//     for (int k = 0; k < N; k++) {

//         localSum += v[globalRow*N + k] * v2[k * N + globalCol];
//         //printf("(%d,%d), values = (%d, %d)\n ", globalRow, globalCol, v[globalRow*N + k], v2[k * N + globalCol]);

//     }
//     //printf("v3[%d]: %d\n", globalRow*K + globalCol, localSum);
//     v3[globalRow*N + globalCol] = localSum;
// }