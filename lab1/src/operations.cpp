#include <iostream>
#include <cmath>
#include "operations.h"

double vect_norm(double* vect, int N) {
    double norm = 0;
    for (int i = 0; i < N; i++) {
        norm += vect[i] * vect[i];
    }
    return std::sqrt(norm);
}

void array_mul(int* src, int* dst, int len, int n) {
    for (int i = 0; i < len; i++) {
        dst[i] = src[i] * n;
    }
}

void print_vect(double* vect, int N) {
    for (int i = 0; i < N; i++) {
        std::cout << vect[i] << "\t";
    }
    std::cout << "\n";
}

void fill_zero(double* array, int size) {
    for (int i = 0; i < size; i++)
        array[i] = 0;
}

void fill_A(double* matrix, int N) {
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            matrix[i * N + j] = (i == j) ? 2 : 1;
        }
    }
}

void fill_b(double* vect, int N) {
    for (int i = 0; i < N; i++) {
        vect[i] = N + 1;
    }
}

void fill_A_part(double* A_part, int N, int line_count, int displ) {
    for (int i = 0; i < line_count; i++) {
        for (int j = 0; j < N; j++) {
            A_part[i * N + j] = (j == i + displ) ? 2 : 1;
        }
    }
}

void fill_b_part(double* b_part, int N, int line_count, int displ) {
    for (int i = 0; i < line_count; i++) {
        b_part[i] = N + 1;
    }
}

void calc_counts(int* counts, int world_size, int num_elements) {
    int base_count = num_elements / world_size;
    int remainder = num_elements - base_count * world_size;

    for (int i = 0; i < world_size; i++) {
        counts[i] = base_count;
        if (i < remainder) {
            counts[i]++;
        }
    }
}

void calc_displs(int* displs, int* send_counts, int world_size) {
    displs[0] = 0;
    for (int i = 1; i < world_size; i++) {
        displs[i] = displs[i - 1] + send_counts[i - 1];
    }
}
