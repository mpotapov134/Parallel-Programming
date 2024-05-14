#define _DEFAULT_SOURCE

#include <stdlib.h>
#include <unistd.h>

#include "task.h"

task_t create_task(int rank, int size, int iter) {
    static int id = 0;
    task_t task;
    task.num_iter = abs(rank - (iter % size));
    task.task_id = id++;
    task.owner_rank = rank;
    return task;
}

void run_task(task_t task) {
    usleep(task.num_iter * 10000);
}
