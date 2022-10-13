#include <iostream>
#include <time.h>
#include <chrono>
#include <fstream>

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

// Perform matrix multiplication.
void matrixMultiplication(int **matrix1, int **matrix2, int **matrix3, int size)
{
    // Multiply mx1 & mx2 to determine mx3.
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

    int size;
    std::cout << "Enter matrix size: ";
    std::cin >> size;

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

    // Perform multiplication.
    matrixMultiplication(mx1, mx2, mx3, size);

    auto stop = std::chrono::high_resolution_clock::now();
    // Converts the duration between start and stop to microseconds to determine run time.
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);

    std::cout << "Duration: " << duration.count();

    // Create results text file.
    std::ofstream matrixResults("SequentialResults.txt");

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