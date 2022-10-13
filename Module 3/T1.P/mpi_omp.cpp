#include <stdio.h>
#include <stdlib.h>
#include <CL/cl.h>
#include <time.h>
#include <chrono>
#include <mpi.h>

using namespace std::chrono;


long SZ = 2500;
int **v, **v2, **v3;

// Free up the memory no longer being used.
void free_memory();

void init(int **&v, int rows, int cols, bool initial);

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

        printf("numtasks: %d.\n", numtasks);

        int process_rows = SZ / numtasks;
        int bcast_elements = (SZ * SZ);
        int local_elements = (SZ * SZ) / numtasks;

        auto start = high_resolution_clock::now();

        MPI_Scatter(&v[0][0], local_elements, MPI_INT, &v[0][0], 0, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Bcast(&v2[0][0], bcast_elements, MPI_INT, 0, MPI_COMM_WORLD);

        // OMP Parallel block
        #pragma omp parallel num_threads(5)

        #pragma omp for 
        for (int i = 0; i < process_rows; i++)
        {
            for (int j = 0; j < SZ; j++)
            {
                int localSum = 0;
                for (int k = 0; k < SZ; k++)
                {
                    // mx1 all row i * mx2 all col j = mx3 row i, col j.
                    localSum += v[i][k] * v2[k][j];
                }
                v3[i][j] = localSum;
            }
        }

        #pragma omp barrier

        MPI_Gather(MPI_IN_PLACE, local_elements, MPI_INT, &v3[0][0], local_elements, MPI_INT, 0, MPI_COMM_WORLD);

        // Store the time (stop time) in a variable
        auto stop = high_resolution_clock::now();

        // Calculates the time difference (duration of execution)
        auto duration = duration_cast<microseconds>(stop - start);

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

        MPI_Scatter(NULL, local_elements, MPI_INT, &v[0][0], local_elements, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Bcast(&v2[0][0], bcast_elements, MPI_INT, 0, MPI_COMM_WORLD);

        // OMP Parallel block
        #pragma omp parallel num_threads(5)

        #pragma omp for
        for (int i = 0; i < process_rows; i++)
        {
            for (int j = 0; j < SZ; j++)
            {
                int localSum = 0;
                for (int k = 0; k < SZ; k++)
                {
                    // mx1 all row i * mx2 all col j = mx3 row i, col j.
                    localSum += v[i][k] * v2[k][j];
                }
                v3[i][j] = localSum;
            }
        }

        // printf("RANK: %d\n", rank);
        // print(v3, process_rows, SZ);

        #pragma omp barrier

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

// Perform matrix multiplication.
void matrixMultiplication(int **matrix1, int **matrix2, int **matrix3, int size)
{
// Multiply mx1 & mx2 to determine mx3.
    #pragma omp for
    for (int i = 0; i < size; i++)
    {
        for (int j = 0; j < size; j++)
        {
            for (int k = 0; k < size; k++)
            {
                // mx1 all row i * mx2 all col j = mx3 row i, col j.
                matrix3[i][j] += matrix1[i][k] * matrix2[k][j];
            }
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
    free(v);
    free(v2);
    free(v3);
}