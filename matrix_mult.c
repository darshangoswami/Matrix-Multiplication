#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <string.h>
#include <math.h>

#define BLOCK_SIZE 128
#define CACHE_LINE 64
#define MIN_SIZE_FOR_PARALLEL 500

typedef struct {
    float* __restrict__ A;
    const float* __restrict__ B;
    const float* __restrict__ C;
    int N;
    int thread_id;
    int num_threads;
    char padding[CACHE_LINE - sizeof(float*) * 3 - sizeof(int) * 3];
} ThreadData __attribute__((aligned(CACHE_LINE)));

float* allocate_matrix_1d(int N) {
    void* tmp;
    size_t size = N * N * sizeof(float);
    if (posix_memalign(&tmp, CACHE_LINE, size)) {
        return NULL;
    }
    return (float*)tmp;
}

void free_matrix_1d(float* matrix) {
    free(matrix);
}

void multiply_block(float* __restrict__ A, 
                   const float* __restrict__ B, 
                   const float* __restrict__ C,
                   const int N, const int i_start, const int i_end) {
    
    float* block_B = aligned_alloc(CACHE_LINE, BLOCK_SIZE * BLOCK_SIZE * sizeof(float));
    float* block_C = aligned_alloc(CACHE_LINE, BLOCK_SIZE * BLOCK_SIZE * sizeof(float));
    
    for (int i0 = i_start; i0 < i_end; i0 += BLOCK_SIZE) {
        const int imax = (i0 + BLOCK_SIZE < i_end) ? i0 + BLOCK_SIZE : i_end;
        
        for (int k0 = 0; k0 < N; k0 += BLOCK_SIZE) {
            const int kmax = (k0 + BLOCK_SIZE < N) ? k0 + BLOCK_SIZE : N;
            
            // Load block of B
            for (int ii = i0; ii < imax; ii++) {
                memcpy(&block_B[(ii - i0) * BLOCK_SIZE], 
                       &B[ii * N + k0], 
                       (kmax - k0) * sizeof(float));
            }
            
            for (int j0 = 0; j0 < N; j0 += BLOCK_SIZE) {
                const int jmax = (j0 + BLOCK_SIZE < N) ? j0 + BLOCK_SIZE : N;
                
                // Load block of C
                for (int kk = k0; kk < kmax; kk++) {
                    memcpy(&block_C[(kk - k0) * BLOCK_SIZE], 
                           &C[kk * N + j0], 
                           (jmax - j0) * sizeof(float));
                }
                
                // Compute block
                for (int ii = i0; ii < imax; ii++) {
                    float* const row_A = &A[ii * N + j0];
                    
                    for (int kk = k0; kk < kmax; kk++) {
                        const float bval = block_B[(ii - i0) * BLOCK_SIZE + (kk - k0)];
                        
                        for (int jj = 0; jj < jmax - j0; jj++) {
                            row_A[jj] += bval * block_C[(kk - k0) * BLOCK_SIZE + jj];
                        }
                    }
                }
            }
        }
    }
    
    free(block_B);
    free(block_C);
}

void serial_multiply(float* A, const float* B, const float* C, int N) {
    memset(A, 0, N * N * sizeof(float));
    multiply_block(A, B, C, N, 0, N);
}

void* parallel_multiply(void* arg) {
    ThreadData* data = (ThreadData*)arg;
    const int N = data->N;
    
    const int chunk_size = ((N + data->num_threads - 1) / data->num_threads + BLOCK_SIZE - 1) 
                          & ~(BLOCK_SIZE - 1);
    const int start_row = data->thread_id * chunk_size;
    const int end_row = (start_row + chunk_size < N) ? start_row + chunk_size : N;
    
    memset(&data->A[start_row * N], 0, (end_row - start_row) * N * sizeof(float));
    multiply_block(data->A, data->B, data->C, N, start_row, end_row);
    return NULL;
}

void print_matrix(const float* matrix, int N) {
    int display_size = N < 5 ? N : 5;
    printf("\n");
    for (int i = 0; i < display_size; i++) {
        for (int j = 0; j < display_size; j++) {
            printf("%.2f\t", matrix[i * N + j]);
        }
        printf(N > display_size ? "...\n" : "\n");
    }
    if (N > display_size) printf("...\n");
}

int verify_results(const float* A, const float* B, int N) {
    float max_diff = 0;
    for (int i = 0; i < N * N; i++) {
        float diff = fabs(A[i] - B[i]);
        max_diff = max_diff > diff ? max_diff : diff;
        if (diff > 0.01) {
            printf("Mismatch at element %d: %.2f vs %.2f\n", i, A[i], B[i]);
            return 0;
        }
    }
    printf("Maximum difference: %.6f\n", max_diff);
    return 1;
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        printf("Usage: %s <matrix_size> <num_threads>\n", argv[0]);
        return 1;
    }

    int N = atoi(argv[1]);
    int num_threads = atoi(argv[2]);
    
    if (N < MIN_SIZE_FOR_PARALLEL) {
        printf("Matrix size %d is too small for parallel execution. Using serial version.\n", N);
        num_threads = 1;
    } else {
        int max_threads = (N + BLOCK_SIZE - 1) / BLOCK_SIZE;
        num_threads = num_threads > max_threads ? max_threads : num_threads;
        num_threads = num_threads > 4 ? 4 : num_threads;
    }
    
    printf("Matrix size: %dx%d\n", N, N);
    printf("Number of threads: %d\n", num_threads);
    printf("Block size: %d\n", BLOCK_SIZE);
    
    float* A_parallel = allocate_matrix_1d(N);
    float* A_serial = allocate_matrix_1d(N);
    float* B = allocate_matrix_1d(N);
    float* C = allocate_matrix_1d(N);
    
    if (!A_parallel || !A_serial || !B || !C) {
        printf("Memory allocation failed\n");
        return 1;
    }
    
    srand(time(NULL));
    for (int i = 0; i < N * N; i++) {
        B[i] = (float)rand() / RAND_MAX * 10.0;
        C[i] = (float)rand() / RAND_MAX * 10.0;
    }
    
    printf("\nInput Matrix B (showing top-left corner):");
    print_matrix(B, N);
    printf("\nInput Matrix C (showing top-left corner):");
    print_matrix(C, N);
    
    clock_t start_serial = clock();
    serial_multiply(A_serial, B, C, N);
    clock_t end_serial = clock();
    double serial_time = ((double)(end_serial - start_serial)) / CLOCKS_PER_SEC;
    
    printf("\nSerial Result Matrix (showing top-left corner):");
    print_matrix(A_serial, N);
    
    clock_t start_parallel = clock();
    
    pthread_t* threads = malloc(num_threads * sizeof(pthread_t));
    ThreadData* thread_data = aligned_alloc(CACHE_LINE, num_threads * sizeof(ThreadData));
    
    if (!threads || !thread_data) {
        printf("Thread memory allocation failed\n");
        return 1;
    }
    
    for (int i = 0; i < num_threads; i++) {
        thread_data[i].A = A_parallel;
        thread_data[i].B = B;
        thread_data[i].C = C;
        thread_data[i].N = N;
        thread_data[i].thread_id = i;
        thread_data[i].num_threads = num_threads;
        
        if (pthread_create(&threads[i], NULL, parallel_multiply, &thread_data[i])) {
            printf("Error creating thread %d\n", i);
            return 1;
        }
    }
    
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }
    
    clock_t end_parallel = clock();
    double parallel_time = ((double)(end_parallel - start_parallel)) / CLOCKS_PER_SEC;
    
    printf("\nParallel Result Matrix (showing top-left corner):");
    print_matrix(A_parallel, N);
    
    printf("\nVerification Results:");
    int correct = verify_results(A_serial, A_parallel, N);
    printf("Verification: %s\n", correct ? "PASSED" : "FAILED");
    
    printf("\nTiming Results:\n");
    printf("Serial time: %.4f seconds\n", serial_time);
    printf("Parallel time: %.4f seconds\n", parallel_time);
    printf("Speedup: %.2fx\n", serial_time / parallel_time);
    
    free(threads);
    free(thread_data);
    free_matrix_1d(A_parallel);
    free_matrix_1d(A_serial);
    free_matrix_1d(B);
    free_matrix_1d(C);
    
    return 0;
}