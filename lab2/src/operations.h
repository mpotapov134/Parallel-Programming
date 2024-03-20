#ifndef OPERATIONS_H_INLUDED
#define OPERATIONS_H_INLUDED

#define THAU 0.0001
#define EPSILON 1.0e-6

double VectNorm(double* vect, int N);

void PrintMat(double* mat, int N);

void PrintVect(double* vect, int N);

void FillZero(double* array, int size);

void FillMatModel(double* matrix, int N);

void FillVectModel(double* vect, int N);

#endif
