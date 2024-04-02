#ifndef LAB3_UTILITY
#define LAB3_UTILITY

enum err_codes {
    VALID = 0,
    INVALID_A_HEIGHT,
    INVALID_B_WIDTH,
    INVALID_SIZE,
};

int check_parameters(int A_height, int grid_height, int B_width, int grid_width, int size);

void print_error(int err_code);

#endif
