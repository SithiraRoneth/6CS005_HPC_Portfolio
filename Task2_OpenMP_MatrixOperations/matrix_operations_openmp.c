#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <omp.h>

typedef struct {
    int rows;
    int cols;
    double **data;
} Matrix;

int max_value(int a, int b) {
    if (a > b) {
        return a;
    }
    return b;
}

int actual_thread_count(int requested_threads, int limit) {
    if (requested_threads < 1) {
        return 1;
    }

    if (limit < 1) {
        return 1;
    }

    if (requested_threads > limit) {
        return limit;
    }

    return requested_threads;
}

Matrix create_matrix(int rows, int cols) {
    Matrix matrix;
    matrix.rows = rows;
    matrix.cols = cols;

    matrix.data = (double **)malloc(rows * sizeof(double *));

    if (matrix.data == NULL) {
        printf("Memory allocation failed.\n");
        exit(1);
    }

    for (int i = 0; i < rows; i++) {
        matrix.data[i] = (double *)malloc(cols * sizeof(double));

        if (matrix.data[i] == NULL) {
            printf("Memory allocation failed.\n");
            exit(1);
        }
    }

    return matrix;
}

void free_matrix(Matrix *matrix) {
    if (matrix->data == NULL) {
        return;
    }

    for (int i = 0; i < matrix->rows; i++) {
        free(matrix->data[i]);
    }

    free(matrix->data);
    matrix->data = NULL;
    matrix->rows = 0;
    matrix->cols = 0;
}

int read_matrix(FILE *input_file, Matrix *matrix, int matrix_number) {
    int rows;
    int cols;

    int header_status = fscanf(input_file, "%d %d", &rows, &cols);

    if (header_status == EOF) {
        return 0;
    }

    if (header_status != 2) {
        printf("Error: Invalid header in matrix %d.\n", matrix_number);
        return -1;
    }

    if (rows <= 0 || cols <= 0) {
        printf("Error: Matrix %d has invalid size.\n", matrix_number);
        return -1;
    }

    *matrix = create_matrix(rows, cols);

    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            if (fscanf(input_file, "%lf", &matrix->data[i][j]) != 1) {
                printf("Error: Invalid or missing value in matrix %d.\n", matrix_number);
                free_matrix(matrix);
                return -1;
            }
        }
    }

    return 1;
}

void write_matrix(FILE *result_file, const char *title, Matrix *matrix) {
    fprintf(result_file, "%s - %d,%d\n", title, matrix->rows, matrix->cols);

    for (int i = 0; i < matrix->rows; i++) {
        for (int j = 0; j < matrix->cols; j++) {
            if (isnan(matrix->data[i][j])) {
                fprintf(result_file, "NaN");
            } else {
                fprintf(result_file, "%.2f", matrix->data[i][j]);
            }

            if (j < matrix->cols - 1) {
                fprintf(result_file, ",");
            }
        }

        fprintf(result_file, "\n");
    }

    fprintf(result_file, "\n");
}

void add_matrices(FILE *result_file, Matrix *A, Matrix *B, int requested_threads) {
    if (A->rows != B->rows || A->cols != B->cols) {
        fprintf(result_file, "Addition cannot be done (shapes differ).\n\n");
        return;
    }

    Matrix result = create_matrix(A->rows, A->cols);
    int threads = actual_thread_count(requested_threads, max_value(A->rows, A->cols));

    #pragma omp parallel for collapse(2) num_threads(threads) schedule(static)
    for (int i = 0; i < A->rows; i++) {
        for (int j = 0; j < A->cols; j++) {
            result.data[i][j] = A->data[i][j] + B->data[i][j];
        }
    }

    write_matrix(result_file, "Addition", &result);
    free_matrix(&result);
}

void subtract_matrices(FILE *result_file, Matrix *A, Matrix *B, int requested_threads) {
    if (A->rows != B->rows || A->cols != B->cols) {
        fprintf(result_file, "Subtraction cannot be done (shapes differ).\n\n");
        return;
    }

    Matrix result = create_matrix(A->rows, A->cols);
    int threads = actual_thread_count(requested_threads, max_value(A->rows, A->cols));

    #pragma omp parallel for collapse(2) num_threads(threads) schedule(static)
    for (int i = 0; i < A->rows; i++) {
        for (int j = 0; j < A->cols; j++) {
            result.data[i][j] = A->data[i][j] - B->data[i][j];
        }
    }

    write_matrix(result_file, "Subtraction", &result);
    free_matrix(&result);
}

void element_multiply_matrices(FILE *result_file, Matrix *A, Matrix *B, int requested_threads) {
    if (A->rows != B->rows || A->cols != B->cols) {
        fprintf(result_file, "Element-by-element multiplication cannot be done (shapes differ).\n\n");
        return;
    }

    Matrix result = create_matrix(A->rows, A->cols);
    int threads = actual_thread_count(requested_threads, max_value(A->rows, A->cols));

    #pragma omp parallel for collapse(2) num_threads(threads) schedule(static)
    for (int i = 0; i < A->rows; i++) {
        for (int j = 0; j < A->cols; j++) {
            result.data[i][j] = A->data[i][j] * B->data[i][j];
        }
    }

    write_matrix(result_file, "Element Multiplication", &result);
    free_matrix(&result);
}

void element_divide_matrices(FILE *result_file, Matrix *A, Matrix *B, int requested_threads) {
    if (A->rows != B->rows || A->cols != B->cols) {
        fprintf(result_file, "Element-by-element division cannot be done (shapes differ).\n\n");
        return;
    }

    Matrix result = create_matrix(A->rows, A->cols);
    int threads = actual_thread_count(requested_threads, max_value(A->rows, A->cols));

    #pragma omp parallel for collapse(2) num_threads(threads) schedule(static)
    for (int i = 0; i < A->rows; i++) {
        for (int j = 0; j < A->cols; j++) {
            if (B->data[i][j] == 0) {
                result.data[i][j] = NAN;
            } else {
                result.data[i][j] = A->data[i][j] / B->data[i][j];
            }
        }
    }

    write_matrix(result_file, "Element Division", &result);
    free_matrix(&result);
}

void transpose_matrix(FILE *result_file, Matrix *matrix, const char *title, int requested_threads) {
    Matrix result = create_matrix(matrix->cols, matrix->rows);
    int threads = actual_thread_count(requested_threads, max_value(result.rows, result.cols));

    #pragma omp parallel for collapse(2) num_threads(threads) schedule(static)
    for (int i = 0; i < matrix->rows; i++) {
        for (int j = 0; j < matrix->cols; j++) {
            result.data[j][i] = matrix->data[i][j];
        }
    }

    write_matrix(result_file, title, &result);
    free_matrix(&result);
}

void multiply_matrices(FILE *result_file, Matrix *A, Matrix *B, int requested_threads) {
    if (A->cols != B->rows) {
        fprintf(result_file, "Matrix multiplication cannot be done (A columns not equal to B rows).\n\n");
        return;
    }

    Matrix result = create_matrix(A->rows, B->cols);
    int threads = actual_thread_count(requested_threads, max_value(result.rows, result.cols));

    #pragma omp parallel for collapse(2) num_threads(threads) schedule(static)
    for (int i = 0; i < A->rows; i++) {
        for (int j = 0; j < B->cols; j++) {
            double sum = 0.0;

            for (int k = 0; k < A->cols; k++) {
                sum = sum + (A->data[i][k] * B->data[k][j]);
            }

            result.data[i][j] = sum;
        }
    }

    write_matrix(result_file, "Matrix Multiplication", &result);
    free_matrix(&result);
}

void process_matrix_pair(FILE *result_file, Matrix *A, Matrix *B, int pair_number, int requested_threads) {
    fprintf(result_file, "========================================\n");
    fprintf(result_file, "Pair %d\n", pair_number);
    fprintf(result_file, "Matrix A Size: %d,%d\n", A->rows, A->cols);
    fprintf(result_file, "Matrix B Size: %d,%d\n", B->rows, B->cols);
    fprintf(result_file, "========================================\n\n");

    add_matrices(result_file, A, B, requested_threads);
    subtract_matrices(result_file, A, B, requested_threads);
    element_multiply_matrices(result_file, A, B, requested_threads);
    element_divide_matrices(result_file, A, B, requested_threads);
    transpose_matrix(result_file, A, "Transpose of A", requested_threads);
    transpose_matrix(result_file, B, "Transpose of B", requested_threads);
    multiply_matrices(result_file, A, B, requested_threads);
}

void create_result_file_path(const char *input_file_path, char *result_file_path, int size) {
    const char *last_slash = strrchr(input_file_path, '/');

    if (last_slash == NULL) {
        snprintf(result_file_path, size, "results.txt");
    } else {
        int folder_length = last_slash - input_file_path + 1;
        snprintf(result_file_path, size, "%.*sresults.txt", folder_length, input_file_path);
    }
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s <input_file_path> <number_of_threads>\n", argv[0]);
        return 1;
    }

    char *input_file_path = argv[1];
    int requested_threads = atoi(argv[2]);

    if (requested_threads <= 0) {
        printf("Error: Number of threads must be greater than 0.\n");
        return 1;
    }

    FILE *input_file = fopen(input_file_path, "r");

    if (input_file == NULL) {
        printf("Error: Cannot open input file.\n");
        return 1;
    }

    char result_file_path[300];
    create_result_file_path(input_file_path, result_file_path, sizeof(result_file_path));

    FILE *result_file = fopen(result_file_path, "w");

    if (result_file == NULL) {
        printf("Error: Cannot create results file.\n");
        fclose(input_file);
        return 1;
    }

    fprintf(result_file, "Multiple Matrix Operations using OpenMP\n");
    fprintf(result_file, "Input File: %s\n", input_file_path);
    fprintf(result_file, "Requested Threads: %d\n\n", requested_threads);

    int pair_number = 1;
    int matrix_number = 1;
    int completed_pairs = 0;

    while (1) {
        Matrix A;
        Matrix B;

        A.data = NULL;
        B.data = NULL;

        int read_A = read_matrix(input_file, &A, matrix_number);

        if (read_A == 0) {
            break;
        }

        if (read_A == -1) {
            fclose(input_file);
            fclose(result_file);
            return 1;
        }

        matrix_number++;

        int read_B = read_matrix(input_file, &B, matrix_number);

        if (read_B == 0) {
            fprintf(result_file, "Matrix %d has no pair. It was skipped.\n", matrix_number - 1);
            free_matrix(&A);
            break;
        }

        if (read_B == -1) {
            free_matrix(&A);
            fclose(input_file);
            fclose(result_file);
            return 1;
        }

        matrix_number++;

        process_matrix_pair(result_file, &A, &B, pair_number, requested_threads);

        free_matrix(&A);
        free_matrix(&B);

        pair_number++;
        completed_pairs++;
    }

    if (completed_pairs == 0) {
        fprintf(result_file, "No complete matrix pairs found.\n");
    }

    fclose(input_file);
    fclose(result_file);

    printf("Task 2 completed successfully.\n");
    printf("Output written to: %s\n", result_file_path);

    return 0;
}
