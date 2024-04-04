#include <algorithm>
#include <chrono>
#include <cmath>
#include <iostream>
#include <string>

#include <mpi.h>

#include "operations.h"

double* find_solution(double* A_part, double* b, int N, int* line_counts, int* line_displs, int rank) {
    int line_count = line_counts[rank];
    int line_displ = line_displs[rank];

    double* x = new double[N];
    double* next_x_part = new double[line_count];
    fill_zero(x, N);
    fill_zero(next_x_part, line_count);

    double res = EPSILON;
    double norm_x_part = 0, norm_x = 0;
    double norm_b = vect_norm(b, N);

    while (res >= EPSILON) {

        for (int i = 0; i < line_count; i++) {
            for (int j = 0; j < N; j++) {
                next_x_part[i] += A_part[i * N + j] * x[j];      // next_x = Ax
            }
            next_x_part[i] -= b[line_displ + i];                 // next_x = Ax - b
            norm_x_part += next_x_part[i] * next_x_part[i];      // ||Ax - b|| ^ 2
            next_x_part[i] *= THAU;                              // next_x = t(Ax - b)
            next_x_part[i] = x[line_displ + i] - next_x_part[i]; // next_x = x - t(Ax - b)
        }

        MPI_Allgatherv(next_x_part, line_count, MPI_DOUBLE, x, line_counts, line_displs,
            MPI_DOUBLE, MPI_COMM_WORLD);
        MPI_Allreduce(&norm_x_part, &norm_x, 1, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);

        res = sqrt(norm_x) / norm_b;
        norm_x_part = 0;
        norm_x = 0;
        fill_zero(next_x_part, line_count);
    }

    delete[] next_x_part;
    return x;
}

int main(int argc, char** argv) {
    std::chrono::high_resolution_clock clock;
    auto start = clock.now();

    if (argc != 2) {
        std::cout << "Missing argument: vector size expected.\n";
        return -1;
    }
    int N = std::stoi(argv[1]);

    MPI_Init(NULL, NULL);

    int world_size, rank;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    int *line_counts = new int[world_size];
    int *line_displs = new int[world_size];
    calc_counts(line_counts, world_size, N);
    calc_displs(line_displs, line_counts, world_size);

    int line_count = line_counts[rank];

    double* A_part = new double[line_count * N];
    fill_A_part(A_part, N, line_counts[rank], line_displs[rank]);

    double* b = new double[N];
    fill_b(b, N);

    double* x = find_solution(A_part, b, N, line_counts, line_displs, rank);

    delete[] A_part;
    delete[] b;
    delete[] x;
    delete[] line_counts;
    delete[] line_displs;
    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Finalize();

    auto end = clock.now();
    auto time = std::chrono::duration_cast<std::chrono::milliseconds> (end - start);
    std::cout << time.count() << " ms\n";
}
