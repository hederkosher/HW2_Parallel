#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <omp.h>
#include <limits.h>
#include <mpi.h>
#include <stdio.h>
#include <string.h>

#define MAX_MATRIX_SIZE 1000
#define MIN_VALUE -100
#define MAX_VALUE 100

// Structure to hold result information
typedef struct {
    int row;
    int col;
    double max_log_product;
} SubmatrixResult;

// Function to generate random matrix
void generate_random_matrix(int **matrix, int N, int M) {
    srand(42); // Fixed seed for reproducibility
    //srand(time(NULL)); // Uncomment for different random values each run
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            matrix[i][j] = (rand() % (MAX_VALUE - MIN_VALUE + 1)) + MIN_VALUE;
            // Ensure we have some odd numbers
            if (matrix[i][j] % 2 == 0 && rand() % 3 == 0) {
                matrix[i][j] += 1;
            }
        }
    }
}

// Function to allocate 2D matrix
int** allocate_matrix(int rows, int cols) {
    int **matrix = (int **)malloc(rows * sizeof(int *));
    if (matrix == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
    }
    
    for (int i = 0; i < rows; i++) {
        matrix[i] = (int *)malloc(cols * sizeof(int));
        if (matrix[i] == NULL) {
            fprintf(stderr, "Memory allocation failed\n");
            exit(1);
        }
    }
    return matrix;
}

// Function to free 2D matrix
void free_matrix(int **matrix, int rows) {
    for (int i = 0; i < rows; i++) {
        free(matrix[i]);
    }
    free(matrix);
}

// Function to check if number is odd
int is_odd(int num) {
    return abs(num) % 2 == 1;
}

// Function to calculate log sum of odd elements in submatrix
double calculate_log_product_submatrix(int **matrix, int start_row, 
                                     int start_col, int K) {
    double log_sum = 0.0;
    int odd_count = 0;
    
    for (int i = start_row; i < start_row + K; i++) {
        for (int j = start_col; j < start_col + K; j++) {
            if (is_odd(matrix[i][j]) && matrix[i][j] != 0) {
                log_sum += log(abs(matrix[i][j]));
                odd_count++;
            }
        }
    }
    
    // Return negative infinity if no odd elements found
    return (odd_count > 0) ? log_sum : -INFINITY;
}

// Function to check if submatrix is valid
int is_valid_submatrix(int i, int j, int K, int N, int M) {
    return (i + K <= N && j + K <= M);
}

// Main function to find best submatrix using OpenMP
SubmatrixResult find_best_submatrix_parallel(int **matrix, int N, int M, int K) {
    SubmatrixResult best_result = {-1, -1, -INFINITY};
    
    // Parallel search using OpenMP
    #pragma omp parallel
    {
        SubmatrixResult local_best = {-1, -1, -INFINITY};
        
        #pragma omp for collapse(2) schedule(dynamic)
        for (int i = 0; i <= N - K; i++) {
            for (int j = 0; j <= M - K; j++) {
                if (is_valid_submatrix(i, j, K, N, M)) {
                    double current_log_product = 
                        calculate_log_product_submatrix(matrix, i, j, K);
                    
                    if (current_log_product > local_best.max_log_product) {
                        local_best.row = i;
                        local_best.col = j;
                        local_best.max_log_product = current_log_product;
                    }
                }
            }
        }
        
        // Critical section to update global best
        #pragma omp critical
        {
            if (local_best.max_log_product > best_result.max_log_product) {
                best_result = local_best;
            }
        }
    }
    
    return best_result;
}

// Function to print matrix (for debugging small matrices)
void print_matrix(int **matrix, int N, int M, int max_print) {
    int print_N = (N > max_print) ? max_print : N;
    int print_M = (M > max_print) ? max_print : M;
    
    printf("Matrix (%dx%d):\n", N, M);
    for (int i = 0; i < print_N; i++) {
        for (int j = 0; j < print_M; j++) {
            printf("%4d ", matrix[i][j]);
        }
        if (M > max_print) printf(" ...");
        printf("\n");
    }
    if (N > max_print) printf("...\n");
    printf("\n");
}

// Function to print submatrix at given position
void print_submatrix(int **matrix, int start_row, int start_col, int K) {
    printf("Submatrix at position (%d, %d):\n", start_row, start_col);
    for (int i = start_row; i < start_row + K; i++) {
        for (int j = start_col; j < start_col + K; j++) {
            printf("%4d ", matrix[i][j]);
            if (is_odd(matrix[i][j])) printf("*");
            else printf(" ");
        }
        printf("\n");
    }
    printf("(* marks odd numbers)\n\n");
}

// Function to validate input parameters
int validate_parameters(int N, int M, int K) {
    if (N <= 0 || M <= 0 || K <= 0) {
        printf("Error: N, M, and K must be positive integers\n");
        return 0;
    }
    
    if (K > N || K > M) {
        printf("Error: K cannot be larger than N or M\n");
        return 0;
    }
    
    if (N > MAX_MATRIX_SIZE || M > MAX_MATRIX_SIZE) {
        printf("Error: Matrix size too large (max %d)\n", MAX_MATRIX_SIZE);
        return 0;
    }
    
    return 1;
}

int main(int argc, char **argv) {
    int N, M, K;
    int **matrix = NULL;
    SubmatrixResult result, local_result;
    double start_time, end_time;
    int rank, size;

    // Initialize MPI
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (size != 2) {
        if (rank == 0)
            printf("Error: This program must be run with exactly 2 MPI processes.\n");
        MPI_Finalize();
        return 1;
    }

    if (rank == 0) {
        // Get input parameters
        printf("Enter matrix dimensions N and M: ");
        scanf("%d %d", &N, &M);

        printf("Enter submatrix size K: ");
        scanf("%d", &K);

        // Validate parameters
        if (!validate_parameters(N, M, K)) {
            MPI_Abort(MPI_COMM_WORLD, 1);
        }

        printf("\nParameters: N=%d, M=%d, K=%d\n", N, M, K);
        printf("Number of OpenMP threads: %d\n", omp_get_max_threads());

        // Allocate and initialize full matrix
        matrix = allocate_matrix(N, M);
        generate_random_matrix(matrix, N, M);

        if (N <= 10 && M <= 10)
            print_matrix(matrix, N, M, 10);
        else
            print_matrix(matrix, N, M, 5);

        start_time = omp_get_wtime();

        // Send matrix dimensions and K
        MPI_Send(&N, 1, MPI_INT, 1, 0, MPI_COMM_WORLD);
        MPI_Send(&M, 1, MPI_INT, 1, 0, MPI_COMM_WORLD);
        MPI_Send(&K, 1, MPI_INT, 1, 0, MPI_COMM_WORLD);

        // Send half the matrix rows to process 1
        int half = N / 2;
        for (int i = half; i < N; i++) {
            MPI_Send(matrix[i], M, MPI_INT, 1, 0, MPI_COMM_WORLD);
        }

        // Process 0 computes on rows 0 to half-1
        local_result = find_best_submatrix_parallel(matrix, half, M, K);

    } else {
        // Receive matrix dimensions and K
        MPI_Recv(&N, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Recv(&M, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Recv(&K, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        int half = N / 2;
        matrix = allocate_matrix(N - half, M);

        for (int i = 0; i < N - half; i++) {
            MPI_Recv(matrix[i], M, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }

        local_result = find_best_submatrix_parallel(matrix, N - half, M, K);

        // Adjust row to global index
        local_result.row += half;

        // Send local result to process 0
        MPI_Send(&local_result, sizeof(SubmatrixResult), MPI_BYTE, 0, 0, MPI_COMM_WORLD);
    }

    if (rank == 0) {
        // Receive result from process 1
        SubmatrixResult other_result;
        MPI_Recv(&other_result, sizeof(SubmatrixResult), MPI_BYTE, 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        // Select better result
        result = (local_result.max_log_product > other_result.max_log_product) ? local_result : other_result;

        end_time = omp_get_wtime();

        if (result.row != -1 && result.col != -1) {
            printf("Best submatrix found at position: (%d, %d)\n", result.row, result.col);
            printf("Log sum of odd elements: %.6f\n", result.max_log_product);

            if (N <= 20 && M <= 20)
                print_submatrix(matrix, result.row, result.col, K);

            printf("Execution time: %.6f seconds\n", end_time - start_time);
        } else {
            printf("No valid submatrix found with odd elements\n");
        }

        free_matrix(matrix, N);
    } else {
        free_matrix(matrix, N - N / 2);
    }

    //Finalize MPI
    MPI_Finalize();
    return 0;
}
