#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

typedef struct matrix {
    int **matrix1;
    int rows1;
    int columns1;
    int **matrix2;
    int rows2;
    int columns2;
    int **tensorproduct;
} matrix_t;

typedef struct thread_data {
    matrix_t *matrices;
    int start_row;
    int end_row;
} thread_data_t;

int** allocate_matrix(int rows, int cols);
void get_matrix_dimensions(const char *filename, int *rows, int *cols);
int** read_matrix(const char *filename, int rows, int cols);
int element_multiptication(int valor, int valoor);
void* tensor_product(void* arg);
void threads_actions(matrix_t *matrices, int num_threads);
int get_max_digit_count(int **matrix, int rows, int cols);
void print_matrix_to_file(int **matrix, int rows, int cols, const char *filename);
void print_matrix(int **matrix, int rows, int cols, const char *filename);
int* max_digit_in_col(int **matrix, int rows, int cols);
void print_formatadio(FILE *file, int **matrix, int rows, int cols, int *max_digits_per_column);

int** allocate_matrix(int rows, int cols) {
    int **matrix = malloc(rows * sizeof(int *));
    if (matrix == NULL) { exit(EXIT_FAILURE); }

    for (int i = 0; i < rows; i++) {
        matrix[i] = malloc(cols * sizeof(int));
        if (matrix[i] == NULL) { exit(EXIT_FAILURE); }
    }
    return matrix;
}

void get_matrix_dimensions(const char *filename, int *rows, int *cols) {
    FILE *file = fopen(filename, "r");
    if (!file) { exit(EXIT_FAILURE); }

    *rows = *cols = 0;
    int temp_cols = 0;
    int ch;
    while ((ch = fgetc(file)) != EOF) {
        if (ch == ' ' || ch == '\t') {
            temp_cols++;
        } else if (ch == '\n') {
            (*rows)++;
            if (*cols == 0) {
                *cols = temp_cols + 1;
            }
            temp_cols = 0;
        }
    }
    if (temp_cols > 0) {
        (*rows)++;
        if (*cols == 0) {
            *cols = temp_cols + 1;
        }
    }
    fclose(file);
}

int** read_matrix(const char *filename, int rows, int cols) {
    FILE *file = fopen(filename, "r");
    if (!file) { exit(EXIT_FAILURE); }

    int **matrix = allocate_matrix(rows, cols);
    
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            fscanf(file, "%d", &matrix[i][j]);
        }
    }
    fclose(file);
    return matrix;
}

int element_multiptication(int valor, int valoor) {
    return valor * valoor;
}

void* tensor_product(void* arg) {
    thread_data_t* data = (thread_data_t*)arg;
    matrix_t* matrices = data->matrices;

    for (int i = data->start_row; i < data->end_row; i++) {
        for (int j = 0; j < matrices->columns1; j++) {
            for (int k = 0; k < matrices->rows2; k++) {
                for (int l = 0; l < matrices->columns2; l++) {
                    int result = element_multiptication(matrices->matrix1[i][j], matrices->matrix2[k][l]);
                    matrices->tensorproduct[i * matrices->rows2 + k][j * matrices->columns2 + l] = result;
                }
            }
        }
    }

    pthread_exit(NULL);
}

void threads_actions(matrix_t *matrices, int num_threads) {
    pthread_t *threads = malloc(num_threads * sizeof(pthread_t));
    thread_data_t *thread_data = malloc(num_threads * sizeof(thread_data_t));
    int rows_per_thread = matrices->rows1 / num_threads;
    int start_row = 0;

    for (int i = 0; i < num_threads; i++) {
        thread_data[i].matrices = matrices;
        thread_data[i].start_row = start_row;
        int rows_to_process = rows_per_thread;
        if (i < matrices->rows1 % num_threads) {
            rows_to_process++;
        }
        thread_data[i].end_row = start_row + rows_to_process;
        start_row = thread_data[i].end_row;

        pthread_create(&threads[i], NULL, tensor_product, &thread_data[i]);
    }

    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    free(threads);
    free(thread_data);
}

void print_matrix(int **matrix, int rows, int cols, const char *filename) {
    FILE *file = fopen(filename, "w");
    if (!file) { exit(EXIT_FAILURE); }

    int *max_digits_per_column = max_digit_in_col(matrix, rows, cols);
    print_formatadio(file, matrix, rows, cols, max_digits_per_column);

    free(max_digits_per_column);
    fclose(file);
}

int* max_digit_in_col(int **matrix, int rows, int cols) {
    int *max_digits_per_column = malloc(cols * sizeof(int));
    if (!max_digits_per_column) { exit(EXIT_FAILURE); }

    for (int j = 0; j < cols; j++) {
        max_digits_per_column[j] = 1;
    }

    for (int j = 0; j < cols; j++) {
        for (int i = 0; i < rows; i++) {
            int num = matrix[i][j];
            int digits = (num == 0) ? 1 : 0;
            while (num != 0) {
                num /= 10;
                digits++;
            }
            if (digits > max_digits_per_column[j]) {
                max_digits_per_column[j] = digits;
            }
        }
    }

    return max_digits_per_column;
}

void print_formatadio(FILE *file, int **matrix, int rows, int cols, int *max_digits_per_column) {
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            fprintf(file, "%*d", max_digits_per_column[j], matrix[i][j]);
            if (j < cols - 1) {
                fprintf(file, " "); 
            }
        }
        fprintf(file, "\n"); 
    }
}



int main(int argc, char *argv[]) {
    if (argc != 3) { return EXIT_FAILURE; }

    matrix_t matrices;

    get_matrix_dimensions(argv[1], &matrices.rows1, &matrices.columns1);
    matrices.matrix1 = read_matrix(argv[1], matrices.rows1, matrices.columns1);
    get_matrix_dimensions(argv[2], &matrices.rows2, &matrices.columns2);
    matrices.matrix2 = read_matrix(argv[2], matrices.rows2, matrices.columns2);

    matrices.tensorproduct = allocate_matrix(matrices.rows1 * matrices.rows2, matrices.columns1 * matrices.columns2);

    int num_threads = 2;
    threads_actions(&matrices, num_threads);

    print_matrix(matrices.tensorproduct, matrices.rows1 * matrices.rows2, matrices.columns1 * matrices.columns2, "tensor.out");

    for (int i = 0; i < matrices.rows1; i++) { free(matrices.matrix1[i]); }
    free(matrices.matrix1);
    for (int i = 0; i < matrices.rows2; i++) { free(matrices.matrix2[i]); }
    free(matrices.matrix2);
    for (int i = 0; i < matrices.rows1 * matrices.rows2; i++) { free(matrices.tensorproduct[i]); }
    free(matrices.tensorproduct);

    return 0;
}
