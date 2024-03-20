#include <iostream>
#include <cmath>
#include "operations.h"

double VectNorm(double* vect, int N) {
    double norm = 0;
    for (int i = 0; i < N; i++) {
        norm += vect[i] * vect[i];
    }
    return std::sqrt(norm);
}

void PrintMat(double* mat, int N) {
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            std::cout << mat[i * N + j] << "\t";
        }
        std::cout << "\n";
    }
}

void PrintVect(double* vect, int N) {
    for (int i = 0; i < N; i++) {
        std::cout << vect[i] << "\n";
    }
}

void FillZero(double* array, int size) {
    for (int i = 0; i < size; i++)
        array[i] = 0;
}

void FillMatModel(double* matrix, int N) {
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            matrix[i * N + j] = (i == j) ? 2 : 1;
        }
    }
}

void FillVectModel(double* vect, int N) {
    for (int i = 0; i < N; i++) {
        vect[i] = N + 1;
    }
}
