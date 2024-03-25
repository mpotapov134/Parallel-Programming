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

        MPI_Gatherv(next_x_part, line_count, MPI_DOUBLE, x, line_counts, line_displs,
            MPI_DOUBLE, 0, MPI_COMM_WORLD);
        MPI_Bcast(x, N, MPI_DOUBLE, 0, MPI_COMM_WORLD);
        MPI_Reduce(&norm_x_part, &norm_x, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
        MPI_Bcast(&norm_x, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);

        res = sqrt(norm_x) / norm_b;
        norm_x_part = 0;
        norm_x = 0;
        fill_zero(next_x_part, line_count);
    }

    delete[] next_x_part;
    return x;
}

int main(int argc, char** argv) {
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

    double *A, *b;
    int *A_send_counts, *A_displs;

    int recv_count;

    if (rank == 0) {
        A = new double[N * N];
        fill_A(A, N);

        A_send_counts = new int[world_size];
        A_displs = new int[world_size];

        array_mul(line_counts, A_send_counts, world_size, N);
        array_mul(line_displs, A_displs, world_size, N);

        recv_count = *std::max_element(A_send_counts, A_send_counts + world_size);
    }

    MPI_Bcast(&recv_count, 1, MPI_INT, 0, MPI_COMM_WORLD);
    double* A_part = new double[recv_count];

    MPI_Scatterv(A, A_send_counts, A_displs, MPI_DOUBLE, A_part, recv_count,
        MPI_DOUBLE, 0, MPI_COMM_WORLD);

    b = new double[N];
    fill_b(b, N);

    std::chrono::high_resolution_clock clock;
    auto start = clock.now();

    double* x = find_solution(A_part, b, N, line_counts, line_displs, rank);

    auto end = clock.now();
    auto time = std::chrono::duration_cast<std::chrono::milliseconds> (end - start);
    std::cout << time.count() << " ms\n";

    if (rank == 0) {
        delete[] A;
        delete[] A_send_counts;
        delete[] A_displs;
    }
    delete[] A_part;
    delete[] b;
    delete[] x;
    delete[] line_counts;
    delete[] line_displs;
    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Finalize();
}
