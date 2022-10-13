#include <mpi.h>
#include <stdio.h>
#include <cstring>

int main(int argc, char** argv) {
    int numtasks, rank, name_len, tag=1; 

    char name[MPI_MAX_PROCESSOR_NAME];
    char messageSend[20] = "Head, Hello Wolrd!";

    MPI_Status status;

    // Initialize the MPI environment
    MPI_Init(&argc,&argv);

    // Get the number of tasks/process
    MPI_Comm_size(MPI_COMM_WORLD, &numtasks);

    // Get the rank
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    // Find the processor name
    MPI_Get_processor_name(name, &name_len);

    if (rank == 0) {

        printf("Process: %d, Sent: %s \n", rank, messageSend);

    }

    MPI_Bcast(&messageSend, 20, MPI_CHAR, 0, MPI_COMM_WORLD);

    printf("Process: %d, Received: %s \n", rank, messageSend);

    // Finalize the MPI environment
    MPI_Finalize();
}