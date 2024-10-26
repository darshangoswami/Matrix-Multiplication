# Parallel Matrix Multiplication Implementation

This program implements matrix multiplication using both serial and parallel approaches with POSIX threads (Pthreads). The parallel implementation uses dynamic task distribution where each thread calculates individual cells of the resultant matrix.

## Project Structure

- `matrix_mult.c`: Main source code containing both serial and parallel implementations
- `Makefile`: For compilation
- `README.md`: This file

## Requirements

- GCC compiler
- POSIX threads library (pthread)
- Linux/Unix environment

## Compilation

```bash
make
```

This will create an executable named `matrix_mult`

## Usage

```bash
./matrix_mult <matrix_size> <num_threads>
```

### Parameters:

- `matrix_size`: Size of the square matrices (N x N)
- `num_threads`: Number of threads to use for parallel computation

### Example:

```bash
./matrix_mult 3 2
```

This will multiply two 3x3 matrices using 2 threads.

## Program Output

The program displays:

1. Thread allocation information showing which thread processes which cell
2. Input matrices B and C
3. Result matrix A from parallel computation
4. Result matrix A from serial computation
5. Verification result comparing parallel and serial outputs

### Sample Output:

```
Starting parallel multiplication with 2 threads...

Thread 0 processing cell [0,0]
Thread 1 processing cell [0,1]
Thread 0 processing cell [0,2]
Thread 1 processing cell [1,0]
...

Parallel multiplication complete.

Matrix size: 3x3
Number of threads: 2

Input Matrix B:
0.84 0.39 0.56
0.12 0.45 0.78
0.91 0.23 0.67

Input Matrix C:
0.34 0.67 0.89
0.21 0.45 0.12
0.56 0.78 0.34

Result Matrix A (Parallel):
0.73 1.24 0.82
0.52 0.89 0.43
0.61 1.12 0.97

Result Matrix A (Serial):
0.73 1.24 0.82
0.52 0.89 0.43
0.61 1.12 0.97

Results verified: Parallel and serial results match!
```

## Implementation Details

1. Serial Implementation:

   - Uses three nested loops for matrix multiplication
   - Results used for verification

2. Parallel Implementation:

   - Uses POSIX threads
   - Dynamic task distribution using mutex
   - Each thread calculates individual cells
   - Thread safe printing of cell assignments

3. Verification:
   - Compares results from parallel and serial computations
   - Uses small tolerance for floating-point comparison

## Memory Management

- All matrices are dynamically allocated
- Proper cleanup of allocated memory
- Mutex destruction handled properly

## Error Handling

- Checks for correct number of command line arguments
- Verifies thread creation success
- Validates matrix multiplication results

## Notes

- Input matrices are initialized with random float values
- The program will display complete matrices regardless of size
- Each cell computation is assigned to threads dynamically

## Clean Up

```bash
make clean
```

This will remove the compiled executable.
