#include <iostream>

#include "utility.h"

int check_parameters(int A_height, int grid_height, int B_width, int grid_width, int size) {
    if (A_height % grid_height != 0) return INVALID_A_HEIGHT;
    if (B_width % grid_width != 0) return INVALID_B_WIDTH;
    if (grid_height * grid_width != size) return INVALID_SIZE;
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
