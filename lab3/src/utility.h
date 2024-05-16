#ifndef LAB3_UTILITY
#define LAB3_UTILITY

enum err_codes {
    VALID = 0,
    INVALID_A_HEIGHT,
    INVALID_B_WIDTH,
    INVALID_SIZE,
};

int check_parameters(int A_num_rows, int grid_num_rows, int B_num_cols, int grid_num_cols, int size);

void print_error(int err_code);

void fill_random(double *buffer, int size);

void print_matrix(double *buffer, int rows, int cols);

#endif
