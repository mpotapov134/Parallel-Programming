#ifndef FUNCTIONS_LAB4_INCLUDED
#define FUNCTIONS_LAB4_INCLUDED

#include "const_defines.h"

void initialize(double *region, int num_layers, int rank, int size);

double calc_next_value(double *region, int x, int y, int z);

#endif
