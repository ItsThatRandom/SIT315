#include <iostream>
#include <time.h>
#include <chrono>
#include <pthread.h>
#include <fstream>

#define THREAD_CNT 12

// Memory allocation for matrix using a pointer to pointers for each row.
int **matrixMalloc(int size)
{
    int **matrix = (int **)malloc(size * sizeof(int *));

    for (int i = 0; i < size; i++)
    {

        // calloc() used to ensure mx3 will have initialised values before the multiplication section.
        matrix[i] = (int *)calloc(size, sizeof(int));
    }
    return matrix;
}

// Initialise the matrices with random values.
void randomMatrix(int **matrix, int size)
{
    for (int i = 0; i < size; i++)
    {
        for (int j = 0; j < size; j++)
        {
            matrix[i][j] = rand() % 5;
        }
    }
}

// Struct for matrix multiplication operations.
struct multiTask
{
    int **matrix1;
    int **matrix2;
    int **matrix3;
    int size;
    int start;
    int end;
};

// Perform matrix multiplication.
void *matrixMultiplication(void *arg)
{
    multiTask *task = ((struct multiTask *)arg);

    // Each thread iterates through matrix row/cols to determine its allocated resulting matrix row values.
    for (int i = task->start; i < task->end; i++)
    {
        for (int j = 0; j < task->size; j++)
        {
            for (int k = 0; k < task->size; k++)
            {
                // mx1 all row i * mx2 all col j = mx3 row i, col j.
                task->matrix3[i][j] += task->matrix1[i][k] * task->matrix2[k][j];
            }
        }
    }

    return NULL;
}

// Loop through matrix and outpute results to txt file.
void outputResults(std::ofstream &matrixResults, int **matrix, int size)
{
    // Matrix output
    for (int i = 0; i < size; i++)
    {
        matrixResults << "[ ";
        for (int j = 0; j < size; j++)
        {
            matrixResults << matrix[i][j] << " ";
        }
        matrixResults << "]" << std::endl;
    }
}

main()
{
    // Determine matrix size.
    int size;
    std::cout << "Enter matrix size: ";
    std::cin >> size;

    // Threads for multiplication operation.
    pthread_t multiThread[THREAD_CNT];

    int **mx1 = matrixMalloc(size);
    int **mx2 = matrixMalloc(size);
    int **mx3 = matrixMalloc(size);

    // Set seed for first matrix and initialise values.
    srand(1);
    randomMatrix(mx1, size);

    // Set seed for second matrix and initialise values.
    srand(2);
    randomMatrix(mx2, size);

    // Sets a time point value to 'start' of the current point in time.
    auto start = std::chrono::high_resolution_clock::now();

    // Set partition size.
    int partition_size = size / THREAD_CNT;

    // Create thread and required struct data to perform matrix multiplication
    for (int i = 0; i < THREAD_CNT; i++)
    {
        struct multiTask *multiTask = (struct multiTask *)malloc(sizeof(struct multiTask));
        // Initialise struct
        multiTask->matrix1 = mx1;
        multiTask->matrix2 = mx2;
        multiTask->matrix3 = mx3;
        multiTask->size = size;
        multiTask->start = i * partition_size;
        multiTask->end = multiTask->start + partition_size;

        // Create new thread for the multiplication use the above details.
        pthread_create(&multiThread[i], NULL, matrixMultiplication, multiTask);
    }

    // Wait for all threads to complete.
    for (int i = 0; i < THREAD_CNT; i++)
    {
        pthread_join(multiThread[i], NULL);
    }

    auto stop = std::chrono::high_resolution_clock::now();

    // Converts the duration between start and stop to microseconds to determine run time.
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);

    std::cout << "Duration: " << duration.count();

    // Create results text file.
    std::ofstream matrixResults("PthreadsResults.txt");

    // Write the output of each matrix to text file.
    matrixResults << "Matrix 1" << std::endl;
    outputResults(matrixResults, mx1, size);

    matrixResults << "Matrix 2" << std::endl;
    outputResults(matrixResults, mx2, size);

    matrixResults << "Matrix 3" << std::endl;
    outputResults(matrixResults, mx3, size);

    matrixResults << "Duration: " << duration.count();

    // Close file.
    matrixResults.close();
}