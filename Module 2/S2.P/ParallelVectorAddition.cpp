#include <iostream>
#include <cstdlib>
#include <time.h>
#include <chrono>
#include <pthread.h>

#define THREAD_CNT 12

using namespace std::chrono;
using namespace std;


// Struct for input in random vector method.
struct RandomTask{
    int seed_f;
    int *v;
    int start;
    int end;
};

// Struct for vector addition method.
struct AdditionTask{
    int *vec1;
    int *vec2;
    int *vec3;
    int start;
    int end;
};

// Random vector method to populate v1/v2.
void* randomVector(void* arg)
{
    RandomTask *task = ((struct RandomTask *)arg);

    // Ensures unique number generation.
    srand(time(0) * task->seed_f);

    for (int i = task->start; i < task->end; i++)
    {
        // Assign random value in range 0 - 99 at inedx 'i' of vector.
        task->v[i] = rand() % 100;
    }

    return NULL;
}

// Vector addition method for creating v3 from v1/v2.
void* vectorAddition(void* arg){

    AdditionTask *task = ((struct AdditionTask *)arg);

    for (int i = task->start; i < task->end; i++) 
    {
        task->vec3[i] = task->vec1[i] + task->vec2[i];
    }

    return NULL;
}

int main(){

    unsigned long size = 1000000;

    int *v1, *v2, *v3;

    // Threads for v1/v2 random vector generation.
    pthread_t randomThread[THREAD_CNT];

    // Threads for v3 vector addtion using v1/v2.
    pthread_t additionThread[THREAD_CNT];

    // Sets a time point value to 'start' of the current point in time.
    auto start = high_resolution_clock::now();

    // Allocate the space required in memory using 'size' with malloc and typcasts it to int, passing the point back to the variables.
    v1 = (int *) malloc(size * sizeof(int *));
    v2 = (int *) malloc(size * sizeof(int *));
    v3 = (int *) malloc(size * sizeof(int *));

    // Split partition size to use half threads on each vector.
    int partition_size = size / (THREAD_CNT/2);

    // Seed factor to ensure unique seed for number generation.
    int seed_f = 12;

    // Perform random vector operation on v1 using half of total threads.
    for(size_t i = 0; i < THREAD_CNT/2; i++) 
    {
        struct RandomTask *randomTask = (struct RandomTask *)malloc(sizeof(struct RandomTask));
        randomTask->seed_f = seed_f++;
        randomTask->v = v1;
        randomTask->start = i * partition_size;
        randomTask->end = randomTask->start + partition_size - 1;

        // Create new thread for the randomVector method using the above details.
        pthread_create(&randomThread[i], NULL, randomVector, randomTask);
    }

    // Perform random vector operation on v2 using second half of total threads.
    for(size_t i = 0; i < THREAD_CNT/2; i++) 
    {
        struct RandomTask *randomTask = (struct RandomTask *)malloc(sizeof(struct RandomTask));
        randomTask->seed_f = seed_f++;
        randomTask->v = v2;
        randomTask->start = i * partition_size;
        randomTask->end = randomTask->start + partition_size - 1;

        // Create new thread for the randomVector method using the above details.
        pthread_create(&randomThread[i], NULL, randomVector, randomTask);
    }


    // Wait for all threads to complete.
    for(size_t i = 0; i < THREAD_CNT; i++) 
    {
        pthread_join(randomThread[i], NULL);
    }

    // Decrease partition size back to using all threads.
    partition_size = partition_size / 2;

    // Repeat process from above but create v3 using vector addition from v1 and v2.
    for (size_t i = 0; i < THREAD_CNT; i++)
    {
        struct AdditionTask *allVectors = (struct AdditionTask *)malloc(sizeof(struct AdditionTask));
        allVectors->vec1 = v1;
        allVectors->vec2 = v2;
        allVectors->vec3 = v3;
        allVectors->start = i * partition_size;
        allVectors->end = allVectors->start + partition_size- 1;

        pthread_create(&additionThread[i], NULL, vectorAddition, allVectors);
    }

    // Wait for all threads to complete.
    for(size_t i = 0; i < THREAD_CNT; i++) 
    {
        pthread_join(additionThread[i], NULL);
    }

    auto stop = high_resolution_clock::now();

    // Converts the duration between start and stop to microseconds to determine run time.
    auto duration = duration_cast<microseconds>(stop - start);
    
    cout << "Time taken by function: "
         << duration.count() << " microseconds" << endl;

    cout << "First 3 values of each vector to ensure working... " << endl;
    cout << v1[0] << " + " << v2[0] << " = " << v3[0] << endl;
    cout << v1[1] << " + " << v2[1] << " = " << v3[1] << endl;
    cout << v1[2] << " + " << v2[2] << " = " << v3[2] << endl;
    return 0;
}