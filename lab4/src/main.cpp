#include <chrono>
#include <cstring>
#include <iostream>
#include <memory>
#include <mpi.h>
#include <utility>

#include "const_defines.h"
#include "functions.h"

static double update_layer(double const *current_region, int layer_z, double *updated_region) {
    double max_delta = 0;
    for (int y = 0; y < Ny; y++) {
        for (int x = 0; x < Nx; x++) {
            int index = layer_z * Nx * Ny + y * Nx + x;

            // Не пересчитываем границу
            if (y == 0 || y == Ny - 1 || x == 0 || x == Nx - 1) {
                updated_region[index] = current_region[index];
                continue;
            }

            updated_region[index] = calc_next_value(current_region, x, y, layer_z);
            double delta = std::abs(current_region[index] - updated_region[index]);
            max_delta = delta > max_delta ? delta : max_delta;
        }
    }
    return max_delta;
}

// Пересчет слоев, которые не зависят от других регионов
static double update_core(double const *current_region, int num_layers, int rank,
        int size, double *updated_region) {

    // Копируем границы области, которые никогда не изменяются
    if (rank == 0) {
        memcpy(updated_region, current_region, Nx * Ny * sizeof(double));
    }
    if (rank == size - 1) {
        memcpy(updated_region + (num_layers - 1) * Nx * Ny,
            current_region + (num_layers - 1) * Nx * Ny, Nx * Ny * sizeof(double));
    }

    int start_layer = 2;
    int end_layer = num_layers - 2;

    double max_delta = 0;
    for (int layer_z = start_layer; layer_z < end_layer; layer_z++) {
        double layer_delta = update_layer(current_region, layer_z, updated_region);
        max_delta = layer_delta > max_delta ? layer_delta : max_delta;
    }
    return max_delta;
}

int main(int argc, char **argv) {
    MPI_Init(NULL, NULL);
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Декомпозиция "по линейке"; разделяем область на регионы, каждый регион
    // состоит из слоев, каждый слой представляет собой плоскость
    int num_layers;
    if (size == 1) {
        num_layers = Nz;
    } else {
        num_layers = (rank == 0 || rank == size - 1) ? Nz / size + 1 : Nz / size + 2;
    }

    auto current_region = std::make_unique<double[]>(Nx * Ny * num_layers);
    auto next_iter = std::make_unique<double[]>(Nx * Ny * num_layers);
    initialize(current_region.get(), num_layers, rank, size);

    std::chrono::high_resolution_clock clock;
    auto start = clock.now();

    int iter = 0;

    double max_delta = EPS;
    while (max_delta >= EPS) {
        iter++;
        MPI_Request requests[4];

        // Отправляем "наверх" и получаем "сверху"
        if (rank != 0) {
            MPI_Isend(current_region.get() + 1 * Nx * Ny, Nx * Ny, MPI_DOUBLE, rank - 1, 0,
                MPI_COMM_WORLD, &requests[0]);
            MPI_Irecv(current_region.get(), Nx * Ny, MPI_DOUBLE, rank - 1, 0,
                MPI_COMM_WORLD, &requests[1]);
        }

        // Отправляем "вниз" и получаем "снизу"
        if (rank != size - 1) {
            MPI_Isend(current_region.get() + (num_layers - 2) * Nx * Ny, Nx * Ny, MPI_DOUBLE,
                rank + 1, 0, MPI_COMM_WORLD, &requests[2]);
            MPI_Irecv(current_region.get() + (num_layers - 1) * Nx * Ny, Nx * Ny, MPI_DOUBLE,
                rank + 1, 0, MPI_COMM_WORLD, &requests[3]);
        }

        double process_delta = update_core(current_region.get(), num_layers, rank, size,
            next_iter.get());

        if (rank != 0) {
            MPI_Wait(&requests[0], MPI_STATUS_IGNORE);
            MPI_Wait(&requests[1], MPI_STATUS_IGNORE);
        }
        if (rank != size - 1) {
            MPI_Wait(&requests[2], MPI_STATUS_IGNORE);
            MPI_Wait(&requests[3], MPI_STATUS_IGNORE);
        }

        if (num_layers >= 3) {
            double layer_delta = update_layer(current_region.get(), num_layers - 2, next_iter.get());
            process_delta = layer_delta > process_delta ? layer_delta : process_delta;
            layer_delta = update_layer(current_region.get(), 1, next_iter.get());
            process_delta = layer_delta > process_delta ? layer_delta : process_delta;
        }

        current_region.swap(next_iter);

        if (iter % 1000 == 0)
            MPI_Allreduce(&process_delta, &max_delta, 1, MPI_DOUBLE, MPI_MAX, MPI_COMM_WORLD);
    }

    auto end = clock.now();
    int64_t time = std::chrono::duration_cast<std::chrono::milliseconds> (end - start).count();
    int64_t max_time;
    MPI_Allreduce(&time, &max_time, 1, MPI_LONG_LONG, MPI_MAX, MPI_COMM_WORLD);
    if (rank == 0) std::cout << max_time << " ms\n";
    if (rank == 0) std::cout << iter << "\n";

    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Finalize();
}
