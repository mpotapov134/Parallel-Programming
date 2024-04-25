#include <iostream>
#include <memory>
#include <mpi.h>

#include "const_defines.h"

static void initialize(int *region, int num_layers, int rank, int size) {
    for (int z = 0; z < num_layers; z++) {
        for (int y = 0; y < Ny; y++) {
            for (int x = 0; x < Nx; x++) {
                int index = z * Nx * Ny + y * Nx + x;

                // Проверка на границу области;
                // верхняя граница - первый слой первого процесса
                // нижняя граница - последний слой последнего процесса
                if ((rank == 0 && z == 0) || (rank == size - 1 && z == num_layers - 1) ||
                        y == 0 || y == Ny - 1 || x == 0 || x == Nx - 1) {
                    region[index] = BORDER;
                } else {
                    region[index] = START;
                }
            }
        }
    }
}

// static void print(int *region, int num_layers) {
//     for (int z = 0; z < num_layers; z++) {
//         for (int y = 0; y < Ny; y++) {
//             for (int x = 0; x < Nx; x++) {
//                 int index = z * Nx * Ny + y * Nx + x;
//                 std::cout << region[index] << "\t";
//             }
//             std::cout << "\n";
//         }
//         std::cout << "============================================================================\n";
//     }
// }

int main(int argc, char **argv) {
    MPI_Init(NULL, NULL);
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Декомпозиция "по линейке"; разделяем область на регионы, каждый регион
    // состоит из слоев, каждый слой представляет собой плоскость
    int num_layers = (rank == 0 || rank == size - 1) ? Nz / size + 1 : Nz / size + 2;
    std::unique_ptr<int[]> region(new int[Nx * Ny * num_layers]);
    initialize(region.get(), num_layers, rank, size);

    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Finalize();
}
