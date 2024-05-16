#include <ctime>
#include <iostream>
#include <cstdlib>

#include "utility.h"

int check_parameters(int A_num_rows, int grid_num_rows, int B_num_cols, int grid_num_cols, int size) {
    if (A_num_rows % grid_num_rows != 0) return INVALID_A_HEIGHT;
    if (B_num_cols % grid_num_cols != 0) return INVALID_B_WIDTH;
    if (grid_num_rows * grid_num_cols != size) return INVALID_SIZE;
    return VALID;
}

void print_error(int err_code) {
    switch (err_code)
    {
    case INVALID_A_HEIGHT:
        std::cerr << "Height of A must be divisible by height of the grid.\n";
        break;
    case INVALID_B_WIDTH:
        std::cerr << "Width of B must be divisible by width of the grid.\n";
        break;
    case INVALID_SIZE:
        std::cerr << "Invalid grid size for this number of processes.\n";
        break;
    default:
        std::cerr << "Unknown error.\n";
        break;
    }
}

void fill_random(double *buffer, int size) {
    srand(clock());
    for (int i = 0; i < size; i++) {
        buffer[i] = (double) (rand() % 100) / 10;
    }
}

void print_matrix(double *buffer, int rows, int cols) {
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            std::cout << buffer[i * cols + j] << "\t";
        }
        std::cout << "\n";
    }
}
