#include <chrono>
#include <cmath>
#include <iostream>
#include <string>

#include "operations.h"

double* Solve(double* A, double* b, int N) {
    double* x = new double[N];
    double* shift_vect = new double[N];
    fill_zero(x, N);
    fill_zero(shift_vect, N);

    double res = EPSILON;
    double norm_x_squared = 0;
    double norm_b = vect_norm(b, N);
    int iter = 0;

    while (res >= EPSILON) {

        for (int i = 0; i < N; i++) {
            for (int j = 0; j < N; j++) {
                shift_vect[i] += A[i * N + j] * x[j];        // shift_vect = Ax
            }
            shift_vect[i] -= b[i];                           // shift_vect = Ax - b
            norm_x_squared += shift_vect[i] * shift_vect[i]; // ||Ax - b|| ^ 2
            shift_vect[i] *= THAU;                           // shift_vect = t(Ax - b)
            shift_vect[i] = x[i] - shift_vect[i];            // x = x - t(Ax - b)
        }

        for (int i = 0; i < N; i++) {
            x[i] = shift_vect[i];
        }

        res = sqrt(norm_x_squared) / norm_b;
        norm_x_squared = 0;
        fill_zero(shift_vect, N);
        iter++;
    }
    std::cout << iter << "\n";

    delete[] shift_vect;
    return x;
}

int main(int argc, char** argv) {
    int N = std::stoi(argv[1]);
    double* A = new double[N * N];
    double* b = new double[N];

    fill_A(A, N);
    fill_b(b, N);

    std::chrono::high_resolution_clock clock;

    auto start = clock.now();
    double* x = Solve(A, b, N);
    auto end = clock.now();

    auto time = std::chrono::duration_cast<std::chrono::milliseconds> (end - start);
    std::cout << time.count() << " ms\n";

    delete[] A;
    delete[] b;
    delete[] x;
}
