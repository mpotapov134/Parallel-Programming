#include <stdio.h>
#include <stdlib.h>

#include "utility.h"

void handle_pthread_error(char *msg, int code) {
    fprintf(stderr, "ERROR %s: %i\n", msg, code);
    exit(EXIT_FAILURE);
}
