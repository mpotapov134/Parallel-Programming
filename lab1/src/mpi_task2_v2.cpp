#include <algorithm>
#include <chrono>
#include <cmath>
#include <iostream>
#include <string>

#include <mpi.h>

#include "operations.h"

void find_solution(double* A_part, double* b_part, double* x_part, int N,
        int max_line_count, int* line_counts, int* line_displs, int rank, int world_size) {

    double* next_x_part = new double[max_line_count]; // для обмена нужен максимальный размер
    fill_zero(next_x_part, max_line_count);

    double res = EPSILON;
    double norm_x = 0, norm_b = 0;
    double norm_x_part = 0, norm_b_part = 0;

    for (int i = 0; i < line_counts[rank]; i++) {
        norm_b_part += b_part[i] * b_part[i];
    }

    MPI_Allreduce(&norm_b_part, &norm_b, 1, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);
    norm_b = sqrt(norm_b);

    while (res >= EPSILON) {
        int current_rank = rank;

        // next_x = Ax
        for (int k = 0; k < world_size; k++) {
            for (int i = 0; i < line_counts[rank]; i++) {
                int shift = line_displs[current_rank];
                for (int j = 0; j < line_counts[current_rank]; j++) {
                    next_x_part[i] += A_part[i * N + shift + j] * x_part[j];
                }
            }

            MPI_Sendrecv_replace(x_part, max_line_count, MPI_DOUBLE, (rank + 1) % world_size,
                0, (world_size + rank - 1) % world_size, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            current_rank = (world_size + current_rank - 1) % world_size;
        }

        for (int i = 0; i < line_counts[rank]; i++) {
            next_x_part[i] -= b_part[i];                    // next_x = Ax - b
            norm_x_part += next_x_part[i] * next_x_part[i]; // ||Ax - b|| ^ 2
            next_x_part[i] *= THAU;                         // next_x = t(Ax - b)
            x_part[i] = x_part[i] - next_x_part[i];         // next_x = x - t(Ax - b)
        }

        MPI_Allreduce(&norm_x_part, &norm_x, 1, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);

        res = sqrt(norm_x) / norm_b;
        norm_x_part = 0;
        norm_x = 0;
        fill_zero(next_x_part, max_line_count);
    }

    delete[] next_x_part;
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
    int max_line_count = *std::max_element(line_counts, line_counts + world_size);

    double* A_part = new double[line_count * N];
    double* b_part = new double[line_count];
    double* x_part = new double[max_line_count]; // для обмена нужен максимальный размер

    fill_A_part(A_part, N, line_counts[rank], line_displs[rank]);
    fill_b_part(b_part, N, line_counts[rank], line_displs[rank]);
    fill_zero(x_part, max_line_count);

    find_solution(A_part, b_part, x_part, N, max_line_count, line_counts, line_displs, rank, world_size);

    double* solution = NULL;
    if (rank == 0) {
        solution = new double[N];
    }
    MPI_Gatherv(x_part, line_counts[rank], MPI_DOUBLE, solution, line_counts, line_displs,
        MPI_DOUBLE, 0, MPI_COMM_WORLD);

    delete[] A_part;
    delete[] b_part;
    delete[] x_part;
    delete[] solution;
    delete[] line_counts;
    delete[] line_displs;
    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Finalize();

    auto end = clock.now();
    auto time = std::chrono::duration_cast<std::chrono::milliseconds> (end - start);
    std::cout << time.count() << " ms\n";
}
