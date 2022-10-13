#include <iostream>
#include <cstdlib>
#include <time.h>
#include <chrono>
#include <cstring>
#include <stdio.h>
#include <mpi.h>


using namespace std::chrono;
using namespace std;

void randomVector(int vector[], int size)
{
    for (int i = 0; i < size; i++)
    {
        // Assign random value in range 0 - 99 at index 'i' of vector.
        vector[i] = rand() % 100;
    }
}


int main(int argc, char** argv){

    unsigned long size = 100000000;
    
    srand(time(0));

    int *v1, *v2, *v3, *sv1, *sv2, *sv3, totalSum;

    int numtasks, rank, name_len, tag=1; 
    char name[MPI_MAX_PROCESSOR_NAME];

    MPI_Status status;

    // Initialize the MPI environment
    MPI_Init(&argc,&argv);

    // Get the number of tasks/process
    MPI_Comm_size(MPI_COMM_WORLD, &numtasks);

    // Get the rank
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    // Find the processor name
    MPI_Get_processor_name(name, &name_len);
    
    // Determine size of partitions to send
    int partition = size/numtasks;

    int localSum = 0;

    // Allocate space for sub vectors using the 'partition' variable from above.
    sv1 = (int *) malloc(partition * sizeof(int *));
    sv2 = (int *) malloc(partition * sizeof(int *));
    sv3 = (int *) malloc(partition * sizeof(int *));

    
    // Specific to rank 0 as it generates the vectors to be scattered amongst other processes.
    if (rank == 0) {
        v1 = (int *) malloc(size * sizeof(int *));
        v2 = (int *) malloc(size * sizeof(int *));
        v3 = (int *) malloc(size * sizeof(int *));
        randomVector(v1, size);
        randomVector(v2, size);
    }

    // Start timer.
    auto start = high_resolution_clock::now();

    // Scatter a partition of each vector across all processes as a sub vector
    MPI_Scatter(v1, partition, MPI_INT, sv1, partition, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Scatter(v2, partition, MPI_INT, sv2, partition, MPI_INT, 0, MPI_COMM_WORLD);

    // Perform the vector addition on the subvectors passed to processes.
    for (int i = 0; i < partition; i++) {
        sv3[i] += sv1[i] + sv2[i];
        localSum += sv3[i];
    }

    // Gather all sv3 partitions to form v3.
    MPI_Gather(sv3, partition, MPI_INT, v3, partition, MPI_INT, 0, MPI_COMM_WORLD);

    // Calculate the total sum of all processes local sums.
    MPI_Reduce(&localSum, &totalSum, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
    
    if (rank == 0) {
        auto stop = high_resolution_clock::now();

        // Convert the duration between start and stop to microseconds to determine run time.
        auto duration = duration_cast<microseconds>(stop - start);
        
        cout << "V3 Total Sum: " << totalSum << ". Time taken by function: " << duration.count() << " microseconds" << endl;
    }

    // Finalize the MPI environment
    MPI_Finalize();
}