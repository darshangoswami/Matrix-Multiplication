#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

typedef struct {
    float** A;  // Result matrix
    float** B;  // First input matrix
    float** C;  // Second input matrix
    int N;      // Matrix dimension
    int thread_id;  // Thread ID
    int num_threads;  // Total number of threads
} ThreadData;

float** allocate_matrix(int N) {
    float** matrix = (float**)malloc(N * sizeof(float*));
    for (int i = 0; i < N; i++) {
        matrix[i] = (float*)malloc(N * sizeof(float));
    }
    return matrix;
}

void free_matrix(float** matrix, int N) {
    for (int i = 0; i < N; i++) {
        free(matrix[i]);
    }
    free(matrix);
}

void serial_multiply(float** A, float** B, float** C, int N) {
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            A[i][j] = 0;
            for (int k = 0; k < N; k++) {
                A[i][j] += B[i][k] * C[k][j];
            }
        }
    }
}

// Modified thread function to compute multiple cells
void* compute_rows(void* arg) {
    ThreadData* data = (ThreadData*)arg;
    int chunk_size = data->N / data->num_threads;
    int start_row = data->thread_id * chunk_size;
    int end_row = (data->thread_id == data->num_threads - 1) ? 
                  data->N : start_row + chunk_size;
    
    // Compute multiple rows
    for (int i = start_row; i < end_row; i++) {
        for (int j = 0; j < data->N; j++) {
            float sum = 0;
            for (int k = 0; k < data->N; k++) {
                sum += data->B[i][k] * data->C[k][j];
            }
            data->A[i][j] = sum;
        }
    }
    return NULL;
}

void print_matrix(float** matrix, int N) {
    int display_size = N < 5 ? N : 5;
    printf("\n");
    for (int i = 0; i < display_size; i++) {
        for (int j = 0; j < display_size; j++) {
            printf("%.2f\t", matrix[i][j]);
        }
        printf(N > display_size ? "...\n" : "\n");
    }
    if (N > display_size) printf("...\n");
}

int compare_matrices(float** A, float** B, int N) {
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            if (abs(A[i][j] - B[i][j]) > 0.001) {
                return 0;
            }
        }
    }
    return 1;
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        printf("Usage: %s <matrix_size> <num_threads>\n", argv[0]);
        return 1;
    }

    int N = atoi(argv[1]);
    int num_threads = atoi(argv[2]);
    
    // Ensure number of threads doesn't exceed matrix size
    if (num_threads > N) {
        num_threads = N;
        printf("Adjusted number of threads to %d\n", num_threads);
    }
    
    printf("Matrix size: %dx%d\n", N, N);
    printf("Number of threads: %d\n", num_threads);
    
    // Allocate matrices
    float** A_parallel = allocate_matrix(N);
    float** A_serial = allocate_matrix(N);
    float** B = allocate_matrix(N);
    float** C = allocate_matrix(N);
    
    // Initialize input matrices
    srand(time(NULL));
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            B[i][j] = (float)rand() / RAND_MAX * 10.0;
            C[i][j] = (float)rand() / RAND_MAX * 10.0;
        }
    }
    
    printf("\nInput Matrix B (showing top-left corner):");
    print_matrix(B, N);
    printf("\nInput Matrix C (showing top-left corner):");
    print_matrix(C, N);
    
    // Serial multiplication
    clock_t start_serial = clock();
    serial_multiply(A_serial, B, C, N);
    clock_t end_serial = clock();
    double serial_time = ((double)(end_serial - start_serial)) / CLOCKS_PER_SEC;
    
    // Parallel multiplication
    clock_t start_parallel = clock();
    
    pthread_t* threads = (pthread_t*)malloc(num_threads * sizeof(pthread_t));
    ThreadData* thread_data = (ThreadData*)malloc(num_threads * sizeof(ThreadData));
    
    // Create threads
    for (int i = 0; i < num_threads; i++) {
        thread_data[i].A = A_parallel;
        thread_data[i].B = B;
        thread_data[i].C = C;
        thread_data[i].N = N;
        thread_data[i].thread_id = i;
        thread_data[i].num_threads = num_threads;
        
        pthread_create(&threads[i], NULL, compute_rows, (void*)&thread_data[i]);
    }
    
    // Wait for all threads
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }
    
    clock_t end_parallel = clock();
    double parallel_time = ((double)(end_parallel - start_parallel)) / CLOCKS_PER_SEC;
    
    // Print results
    printf("\nParallel Result Matrix (showing top-left corner):");
    print_matrix(A_parallel, N);
    
    // Verify results
    int correct = compare_matrices(A_serial, A_parallel, N);
    printf("\nVerification: %s\n", correct ? "PASSED" : "FAILED");
    
    // Print timing results
    printf("\nTiming Results:\n");
    printf("Serial time: %.4f seconds\n", serial_time);
    printf("Parallel time: %.4f seconds\n", parallel_time);
    printf("Speedup: %.2fx\n", serial_time / parallel_time);
    
    // Free resources
    free(threads);
    free(thread_data);
    free_matrix(A_parallel, N);
    free_matrix(A_serial, N);
    free_matrix(B, N);
    free_matrix(C, N);
    
    return 0;
}