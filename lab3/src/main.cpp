#include <chrono>
#include <iostream>
#include <memory>
#include <string>

#include <mpi.h>

#include "utility.h"

void multiply(double *A_part, double *B_part, int A_part_rows, int A_part_cols,
        int B_part_rows, int B_part_cols, double *C_part) {
    memset(C_part, 0, sizeof(*C_part) * A_part_rows * B_part_cols);
    for (int i = 0; i < A_part_rows; i++) {
        for (int j = 0; j < B_part_rows; j++) {
            for (int k = 0; k < B_part_cols; k++) {
                C_part[i * B_part_cols + k] += A_part[i * A_part_cols + j] * B_part[j * B_part_cols + k];
            }
        }
    }
}

int main(int argc, char** argv) {
    if (argc != 6) {
        std::cerr << "Incorrenct number of arguments.\n";
        return -1;
    }

    int A_num_rows = std::stoi(argv[1]);
    int A_num_cols = std::stoi(argv[2]);
    int B_num_rows = A_num_cols;
    int B_num_cols = std::stoi(argv[3]);
    int grid_num_rows = std::stoi(argv[4]);
    int grid_num_cols = std::stoi(argv[5]);

    MPI_Init(NULL, NULL);

    int rank, grid_size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &grid_size);

    int result_of_check = check_parameters(A_num_rows, grid_num_rows, B_num_cols, grid_num_cols, grid_size);
    if (result_of_check != VALID) {
        if (rank == 0) print_error(result_of_check);
        MPI_Abort(MPI_COMM_WORLD, -1);
    }

    std::unique_ptr<double> A, B;
    if (rank == 0) {
        A.reset(new double[A_num_rows * A_num_cols]);
        B.reset(new double[B_num_rows * B_num_cols]);
        fill_random(A.get(), A_num_rows * A_num_cols);
        fill_random(B.get(), B_num_rows * B_num_cols);
    }

    std::chrono::high_resolution_clock clock;
    auto start = clock.now();

    MPI_Comm grid_comm;
    int dims[2] = {grid_num_rows, grid_num_cols};
    int periods[2] = {0, 0};
    MPI_Cart_create(MPI_COMM_WORLD, 2, dims, periods, 0, &grid_comm);

    int rank_in_grid;
    MPI_Comm_rank(grid_comm, &rank_in_grid);
    int coords_in_grid[2];
    MPI_Cart_coords(grid_comm, rank_in_grid, 2, coords_in_grid);

    MPI_Comm rows_comm, cols_comm;
    int remain_dims[2] = {1, 0};
    MPI_Cart_sub(grid_comm, remain_dims, &rows_comm);

    remain_dims[0] = 0;
    remain_dims[1] = 1;
    MPI_Cart_sub(grid_comm, remain_dims, &cols_comm);

    int num_rows_for_each = A_num_rows / grid_num_rows;
    std::unique_ptr<double> A_part(new double[num_rows_for_each * A_num_cols]);

    if (coords_in_grid[1] == 0) { // Проверяем, что процесс находится в первом столбце
        MPI_Scatter(A.get(), num_rows_for_each * A_num_cols, MPI_DOUBLE, A_part.get(),
            num_rows_for_each * A_num_cols, MPI_DOUBLE, 0, rows_comm);
    }

    MPI_Bcast(A_part.get(), num_rows_for_each * A_num_cols, MPI_DOUBLE, 0, cols_comm);

    int num_cols_for_each = B_num_cols / grid_num_cols;
    std::unique_ptr<double> B_part(new double[B_num_rows * num_cols_for_each]);

    if (coords_in_grid[0] == 0) { // Проверяем, что процесс находится в первой строке
        MPI_Datatype mpi_vector_B;
        MPI_Type_vector(B_num_rows, num_cols_for_each, B_num_cols, MPI_DOUBLE, &mpi_vector_B);
        MPI_Type_create_resized(mpi_vector_B, 0, num_cols_for_each * sizeof(double), &mpi_vector_B);
        MPI_Type_commit(&mpi_vector_B);

        MPI_Scatter(B.get(), 1, mpi_vector_B, B_part.get(),
            B_num_rows * num_cols_for_each, MPI_DOUBLE, 0, cols_comm);
        MPI_Type_free(&mpi_vector_B);
    }

    MPI_Bcast(B_part.get(), B_num_rows * num_cols_for_each, MPI_DOUBLE, 0, rows_comm);

    std::unique_ptr<double> C_part(new double[num_rows_for_each * num_cols_for_each]);
    multiply(A_part.get(), B_part.get(), num_rows_for_each, A_num_cols, B_num_rows, num_cols_for_each, C_part.get());

    MPI_Datatype mpi_vector_C;
    MPI_Type_vector(num_rows_for_each, num_cols_for_each, B_num_cols, MPI_DOUBLE, &mpi_vector_C);
    MPI_Type_create_resized(mpi_vector_C, 0, num_cols_for_each * sizeof(double), &mpi_vector_C);
    MPI_Type_commit(&mpi_vector_C);

    std::unique_ptr<int> recv_counts(new int[grid_size]);
    std::fill(recv_counts.get(), recv_counts.get() + grid_size, 1);

    std::unique_ptr<int> displs(new int[grid_size]);
    for (int i = 0; i < grid_num_rows; i++) {
        displs.get()[i * grid_num_cols] = i * num_rows_for_each * grid_num_cols;
        for (int j = 1; j < grid_num_cols; j++) {
            int cur_index = i * grid_num_cols + j;
            displs.get()[cur_index] = displs.get()[cur_index - 1] + recv_counts.get()[cur_index - 1];
        }
    }

    std::unique_ptr<double> C;
    if (rank == 0) {
        C.reset(new double[A_num_rows * B_num_cols]);
    }

    MPI_Gatherv(C_part.get(), num_rows_for_each * num_cols_for_each, MPI_DOUBLE, C.get(), recv_counts.get(),
        displs.get(), mpi_vector_C, 0, MPI_COMM_WORLD);
    MPI_Type_free(&mpi_vector_C);

    auto end = clock.now();
    auto time = std::chrono::duration_cast<std::chrono::milliseconds> (end - start);
    if (rank == 0) std::cout << time.count() << " ms\n";

    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Finalize();
}
