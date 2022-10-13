#include <iostream>
#include <cstdlib>
#include <time.h>
#include <chrono>
#include <omp.h>

using namespace std::chrono;
using namespace std;

void randomVector(int vector[], int size)
{
    // OMP for loop randomising vector values.
    #pragma omp for
    for (int i = 0; i < size; i++)
    {
        // printf("\n%d", size);
        vector[i] = rand() % 100;
    }
}

int main(){

    unsigned long size = 1000000;

    srand(time(0));

    int *v1, *v2, *v3;
    int total = 0;
    int reducTotal = 0;
    int critTotal = 0;

    // Allocate the space required in memory using 'size' with malloc and typcasts it to int, passing the point back to the variables.
    v1 = (int *) malloc(size * sizeof(int *));
    v2 = (int *) malloc(size * sizeof(int *));
    v3 = (int *) malloc(size * sizeof(int *));

    // Sets a time point value to 'start' of the current point in time.
    auto start = high_resolution_clock::now();
    // OMP parallel block
    #pragma omp parallel default(none) firstprivate(size) shared(v1, v2, v3)
    {
        randomVector(v1, size);
        randomVector(v2, size);

        #pragma omp barrier

        #pragma omp for
        for (int i = 0; i < size; i++)
        {
            v3[i] = v1[i] + v2[i];
        }

        /////////////////////////////////////////////////////////////////////////////////
        //                                   Atomic Section                            //
        /////////////////////////////////////////////////////////////////////////////////

        // #pragma omp for
        // for (int i = 0; i < size; i++)
        // {
        //     v3[i] = v1[i] + v2[i];

        //     // Atomic update total value of all V3 elements.
        //     #pragma omp atomic update
        //     total += v3[i];
        // }

        /////////////////////////////////////////////////////////////////////////////////
        //                              Critical Section                               //
        /////////////////////////////////////////////////////////////////////////////////

        // // Private variable
        // int privTotal = 0;

        // // OMP for loop completing vector addition
        // #pragma omp for schedule(static)
        // for (int i = 0; i < size; i++)
        // {
        //     v3[i] = v1[i] + v2[i];

        //     privTotal += v3[i];
        // }

        // // Critical section
        // #pragma omp critical
        // {
        //     critTotal += privTotal;
        // }

        /////////////////////////////////////////////////////////////////////////////////
        //                               Reduction Section                             //
        /////////////////////////////////////////////////////////////////////////////////

        // #pragma omp parallel for reduction (+ : reducTotal)
        // for (int i = 0; i < size; i++)
        // {
        //     reducTotal += v3[i];
        // }

        /////////////////////////////////////////////////////////////////////////////////
        //                              Reduction & Atomic Section                     //
        /////////////////////////////////////////////////////////////////////////////////

        // #pragma omp parallel for reduction (+ : total)
        // for (int i = 0; i < size; i++)
        // {
        //     v3[i] = v1[i] + v2[i];

        //     #pragma omp atomic update
        //     total += v3[i];
        // }
    }

    auto stop = high_resolution_clock::now();

    // Converts the duration between start and stop to microseconds to determine run time.
    auto duration = duration_cast<microseconds>(stop - start);


    cout << "Time taken by function: "
         << duration.count() << " microseconds" << endl;

    // cout << "V1[0] = " << v1[0] << "\nV2[0] = " << v2[0] << "\nv3[0]" << v3[0] << endl;
    cout << "V3 Atomic Total: " << total << endl;
    cout << "V3 Reduction Total: " << reducTotal << endl;
    cout << "V3 Critical Total: " << critTotal << endl;
    return 0;
}