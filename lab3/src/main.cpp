#include <iostream>
#include <string>

#include <mpi.h>

#include "utility.h"

int main(int argc, char** argv) {
    if (argc != 6) {
        std::cerr << "Incorrenct number of arguments.\n";
        return -1;
    }

    int A_height = std::stoi(argv[1]);
    int A_width = std::stoi(argv[2]);
    int B_height = A_width;
    int B_width = std::stoi(argv[3]);
    int grid_height = std::stoi(argv[4]);
    int grid_width = std::stoi(argv[5]);

    MPI_Init(NULL, NULL);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int result_of_check = check_parameters(A_height, grid_height, B_width, grid_width, size);
    if (result_of_check != VALID) {
        if (rank == 0) print_error(result_of_check);
        MPI_Abort(MPI_COMM_WORLD, -1);
    }

    MPI_Comm grid_comm;
    int dims[2] = {grid_height, grid_width};
    int periods[2] = {0, 0};
    MPI_Cart_create(MPI_COMM_WORLD, 2, dims, periods, 0, &grid_comm);

    MPI_Comm lines_comm, columns_comm;
    int remain_dims[2] = {1, 0}; // Каждой строке сопоставляется один rank, столбцы склеиваем
    MPI_Cart_sub(grid_comm, remain_dims, &lines_comm);

    remain_dims[0] = 0;
    remain_dims[1] = 1; // Каждому столбцу сопоставляется один rank, строки склеиваем
    MPI_Cart_sub(grid_comm, remain_dims, &columns_comm);

    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Finalize();
}
