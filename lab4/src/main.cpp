#include <iostream>
#include <memory>
#include <mpi.h>
#include <utility>

#include "const_defines.h"
#include "functions.h"

static double update_layer(std::unique_ptr<double[]> region, int num_layers, int rank, int size) {
    // Вычисления будем выполнять в слоях [start_layer; end_layer]
    // Слой 0 - либо граница, либо слой предыдущего процесса, его не берем
    // Последнй слой - либо граница, либо слой следующего процесса, его не берем
    int start_layer = 1;
    int end_layer = num_layers - 2;

    std::unique_ptr<double[]> new_region(new double[Nx * Ny * num_layers]);
    double max_delta = 0;

    for (int z = start_layer; z <= end_layer; z++) {
        for (int y = 1; y < Ny - 1; y++) {
            for (int x = 1; x < Nx - 1; x++) {
                int index = z * Nx * Ny + y * Nx + x;
                new_region[index] = calc_next_value(region.get(), x, y, z);
                double delta = std::abs(region[index] - new_region[index]);
                max_delta = delta > max_delta ? delta : max_delta;
            }
        }
    }

    region = std::move(new_region);
    return max_delta;
}

int main(int argc, char **argv) {
    MPI_Init(NULL, NULL);
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Декомпозиция "по линейке"; разделяем область на регионы, каждый регион
    // состоит из слоев, каждый слой представляет собой плоскость
    int num_layers = (rank == 0 || rank == size - 1) ? Nz / size + 1 : Nz / size + 2;
    std::unique_ptr<double[]> region(new double[Nx * Ny * num_layers]);
    initialize(region.get(), num_layers, rank, size);

    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Finalize();
}
