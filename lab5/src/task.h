#ifndef COMPUTATION_TASK_DEFINED
#define COMPUTATION_TASK_DEFINED

typedef struct {
    int num_iter;
    int task_id;
    int owner_rank;
} task_t;

task_t create_task(int rank, int size, int iter);

void run_task(task_t task);

#endif
