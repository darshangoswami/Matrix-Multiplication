#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <string.h>
#include <math.h>
#include <unistd.h>  // Added for gettid()

// Structure to pass data to threads
typedef struct {
    int row;
    int col;
    int N;
    float *A;
    float *B;
    float *C;
    int thread_id;  // Added thread ID
} ThreadData;

// Global mutex for task distribution
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t print_mutex = PTHREAD_MUTEX_INITIALIZER;  // Added for synchronized printing
int current_task = 0;
int total_tasks;

// Function prototypes
void multiply_matrices_serial(float *A, float *B, float *C, int N);
void *thread_multiply(void *arg);
void print_matrix(float *matrix, int N);
int get_next_task(int *row, int *col, int N);
int compare_matrices(float *A, float *B, int N);

// Serial matrix multiplication
void multiply_matrices_serial(float *A, float *B, float *C, int N) {
    for(int i = 0; i < N; i++) {
        for(int j = 0; j < N; j++) {
            float sum = 0.0;
            for(int k = 0; k < N; k++) {
                sum += B[i * N + k] * C[k * N + j];
            }
            A[i * N + j] = sum;
        }
    }
}

// Get next task for a thread
int get_next_task(int *row, int *col, int N) {
    pthread_mutex_lock(&mutex);
    if (current_task >= total_tasks) {
        pthread_mutex_unlock(&mutex);
        return 0;
    }
    *row = current_task / N;
    *col = current_task % N;
    current_task++;
    pthread_mutex_unlock(&mutex);
    return 1;
}

// Thread function for parallel multiplication
void *thread_multiply(void *arg) {
    ThreadData *data = (ThreadData *)arg;
    int row, col;
    
    while(get_next_task(&row, &col, data->N)) {
        float sum = 0.0;
        for(int k = 0; k < data->N; k++) {
            sum += data->B[row * data->N + k] * data->C[k * data->N + col];
        }
        data->A[row * data->N + col] = sum;
        
        // Print which thread is processing which cell
        pthread_mutex_lock(&print_mutex);
        printf("Thread %d processing cell [%d,%d]\n", data->thread_id, row, col);
        pthread_mutex_unlock(&print_mutex);
    }
    
    return NULL;
}

// Print matrix function
void print_matrix(float *matrix, int N) {
    for(int i = 0; i < N; i++) {
        for(int j = 0; j < N; j++) {
            printf("%.2f ", matrix[i * N + j]);
        }
        printf("\n");
    }
    printf("\n");
}

// Compare matrices function
int compare_matrices(float *A, float *B, int N) {
    for(int i = 0; i < N * N; i++) {
        if(fabsf(A[i] - B[i]) > 0.0001) {
            return 0;
        }
    }
    return 1;
}

int main(int argc, char *argv[]) {
    if(argc != 3) {
        printf("Usage: %s <matrix_size> <num_threads>\n", argv[0]);
        return 1;
    }

    int N = atoi(argv[1]);
    int num_threads = atoi(argv[2]);
    total_tasks = N * N;

    // Allocate matrices
    float *A = (float *)malloc(N * N * sizeof(float));
    float *B = (float *)malloc(N * N * sizeof(float));
    float *C = (float *)malloc(N * N * sizeof(float));
    float *A_serial = (float *)malloc(N * N * sizeof(float));

    // Initialize matrices B and C with random values
    srand(time(NULL));
    for(int i = 0; i < N * N; i++) {
        B[i] = (float)rand() / RAND_MAX;
        C[i] = (float)rand() / RAND_MAX;
    }

    // Create threads array and thread_data array
    pthread_t *threads = (pthread_t *)malloc(num_threads * sizeof(pthread_t));
    ThreadData *thread_data = (ThreadData *)malloc(num_threads * sizeof(ThreadData));

    printf("Starting parallel multiplication with %d threads...\n\n", num_threads);
    
    // Create and execute threads
    for(int i = 0; i < num_threads; i++) {
        thread_data[i] = (ThreadData){0, 0, N, A, B, C, i};
        pthread_create(&threads[i], NULL, thread_multiply, &thread_data[i]);
    }

    // Wait for all threads to complete
    for(int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    printf("\nParallel multiplication complete.\n\n");

    // Perform serial multiplication for verification
    multiply_matrices_serial(A_serial, B, C, N);

    // Print results
    printf("Matrix size: %dx%d\n", N, N);
    printf("Number of threads: %d\n\n", num_threads);
    
    printf("Input Matrix B:\n");
    print_matrix(B, N);
    printf("Input Matrix C:\n");
    print_matrix(C, N);
    printf("Result Matrix A (Parallel):\n");
    print_matrix(A, N);
    printf("Result Matrix A (Serial):\n");
    print_matrix(A_serial, N);

    // Verify results
    if(compare_matrices(A, A_serial, N)) {
        printf("Results verified: Parallel and serial results match!\n");
    } else {
        printf("ERROR: Results do not match!\n");
    }

    // Cleanup
    free(A);
    free(B);
    free(C);
    free(A_serial);
    free(threads);
    free(thread_data);
    pthread_mutex_destroy(&mutex);
    pthread_mutex_destroy(&print_mutex);

    return 0;
}