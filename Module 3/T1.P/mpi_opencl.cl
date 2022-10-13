__kernel void multiply_matrices(const int M, const int N, const int K,
                      const __global int* v,
                      const __global int* v2,
                      __global int* v3) {
    
    // Thread identifierss
    const int globalRow = get_global_id(0); // Row ID of C (0..M)
    const int globalCol = get_global_id(1); // Col ID of C (0..N)
    //printf("M: %d, N: %d, K: %d\n", M, N, K);
    
    // Compute a single element (loop over K)
    
    
    int localSum = 0;
    for (int k = 0; k < N; k++) {

        localSum += v[globalRow*N + k] * v2[k * N + globalCol];
        //printf("(%d,%d), values = (%d, %d)\n ", globalRow, globalCol, v[globalRow*N + k], v2[k * N + globalCol]);

    }
    //printf("v3[%d]: %d\n", globalRow*K + globalCol, localSum);
    v3[globalRow*N + globalCol] = localSum;
}