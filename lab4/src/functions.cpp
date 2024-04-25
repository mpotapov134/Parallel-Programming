#include "functions.h"

void initialize(double *region, int num_layers, int rank, int size) {
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

double calc_next_value(double *region, int x, int y, int z) {
    static const double hx_sq = (Dx / (Nx - 1)) * (Dx / (Nx - 1));
    static const double hy_sq = (Dy / (Ny - 1)) * (Dy / (Ny - 1));
    static const double hz_sq = (Dz / (Nz - 1)) * (Dz / (Nz - 1));
    static const double multiplier = 1 / (2 / hx_sq + 2 / hy_sq + 2 / hz_sq + a);

    double sum_prev =
        (region[(z - 1) * Nx * Ny + y * Nx + x] + region[(z + 1) * Nx * Ny + y * Nx + x]) / hz_sq +
        (region[z * Nx * Ny + (y - 1) * Nx + x] + region[z * Nx * Ny + (y + 1) * Nx + x]) / hy_sq +
        (region[z * Nx * Ny + y * Nx + (x - 1)] + region[z * Nx * Ny + y * Nx + (x + 1)]) / hx_sq -
        (6 - a * region[z * Nx * Ny + y * Nx + x]); // r(x, y, z)

    return multiplier * sum_prev;
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
