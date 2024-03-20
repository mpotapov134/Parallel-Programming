#ifndef OPERATIONS_H_INLUDED
#define OPERATIONS_H_INLUDED

#define THAU 0.0001
#define EPSILON 1.0e-6

double vect_norm(double* vect, int N);

void array_mul(int* src, int* dst, int len, int n);

void print_vect(double* vect, int N);

void fill_zero(double* array, int size);

void fill_A(double* matrix, int N);

void fill_b(double* vect, int N);

void calc_counts(int* line_counts, int world_size, int num_elements);

void calc_displs(int* displs, int* send_counts, int world_size);

#endif
