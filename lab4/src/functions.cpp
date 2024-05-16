#include <iostream>

#include "functions.h"

static double calc_border(int x, int y, int z) {
    static const double hx = (double) Dx / (Nx - 1);
    static const double hy = (double) Dy / (Ny - 1);
    static const double hz = (double) Dz / (Nz - 1);
    return (x0 + x * hx) * (x0 + x * hx) + (y0 + y * hy) * (y0 + y * hy) +
        (z0 + z * hz) * (z0 + z * hz);
}

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
                    int absolute_z = Nz / size * rank + z;
                    region[index] = calc_border(x, y, absolute_z);
                } else {
                    region[index] = START;
                }
            }
        }
    }
}

double calc_next_value(double const *region, int x, int y, int z) {
    static const double hx_sq = ((double) Dx / (Nx - 1)) * ((double) Dx / (Nx - 1));
    static const double hy_sq = ((double) Dy / (Ny - 1)) * ((double) Dy / (Ny - 1));
    static const double hz_sq = ((double) Dz / (Nz - 1)) * ((double) Dz / (Nz - 1));
    static const double multiplier = 1 / (2 / hx_sq + 2 / hy_sq + 2 / hz_sq + a);

    double sum_prev =
        (region[(z - 1) * Nx * Ny + y * Nx + x] + region[(z + 1) * Nx * Ny + y * Nx + x]) / hz_sq +
        (region[z * Nx * Ny + (y - 1) * Nx + x] + region[z * Nx * Ny + (y + 1) * Nx + x]) / hy_sq +
        (region[z * Nx * Ny + y * Nx + (x - 1)] + region[z * Nx * Ny + y * Nx + (x + 1)]) / hx_sq -
        (6 - a * region[z * Nx * Ny + y * Nx + x]); // r(x, y, z)

    return multiplier * sum_prev;
}

void print(double *region, int num_layers) {
    for (int z = 0; z < num_layers; z++) {
        for (int y = 0; y < Ny; y++) {
            for (int x = 0; x < Nx; x++) {
                int index = z * Nx * Ny + y * Nx + x;
                printf("%8.4f ", region[index]);
            }
            std::cout << "\n";
        }
        std::cout << "============================================================================\n";
    }
}
