# Parallel Matrix Multiplication

This program implements both serial and parallel matrix multiplication using POSIX threads (Pthreads).

## Features

- Parallel matrix multiplication using dynamic task distribution
- Serial implementation for result verification
- Performance comparison between parallel and serial versions
- Support for arbitrary matrix sizes and number of threads

## Compilation

To compile the program, simply run:

```bash
make
```

## Usage

Run the program with two command-line arguments:

```bash
./matrix_mult <matrix_size> <num_threads>
```

Example:

```bash
./matrix_mult 1000 4
```

This will multiply two 1000x1000 matrices using 4 threads.

## Implementation Details

- Each thread is dynamically assigned cells to compute
- Uses mutex locks for thread synchronization
- Compares parallel results with serial computation for verification
- Displays execution time and speedup metrics

## Notes

- For matrices larger than 10x10, the program will not display the actual matrices to avoid cluttering the output
- The program uses floating-point numbers for matrix elements
- Random values are used to initialize the input matrices
