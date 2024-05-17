#define _DEFAULT_SOURCE

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "blocking_queue.h"
#include "task.h"
#include "utility.h"

#define TASK_REQUEST 1
#define TASK_RESPONSE 2

#define NUM_TASKS 100
#define NUM_ROUNDS 5
#define SLEEP_TIME 10000

static MPI_Datatype mpi_task;
static int work_completed;

void execute_tasks(b_queue_t *task_queue) {
    int proc_rank, comm_size;
    MPI_Comm_rank(MPI_COMM_WORLD, &proc_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_size);

    while (1) {

        /* Execute available tasks */
        while (1) {
            task_t task;
            if (queue_get(task_queue, &task) == NULL)
                break; // no tasks available

            run_task(task);
            // printf("Task %i from node %i completed on node %i\n",
            //     task.task_id, task.owner_rank, proc_rank);
        }

        // for (int i = 1; i < comm_size; i++) {
        //     int polled_node = (proc_rank + i) % comm_size;
        //     task_t task_received;
        //     MPI_Status recv_status;

        //     MPI_Send(NULL, 0, MPI_INT, polled_node, TASK_REQUEST, MPI_COMM_WORLD);
        //     MPI_Recv(&task_received, 1, mpi_task, polled_node, TASK_RESPONSE,
        //         MPI_COMM_WORLD, &recv_status);

        //     int num_tasks_received;
        //     MPI_Get_count(&recv_status, mpi_task, &num_tasks_received);
        //     if (num_tasks_received == 1) {
        //         queue_put(task_queue, task_received);
        //         break;
        //     }
        // }

        if (queue_is_empty(task_queue)) {
            MPI_Barrier(MPI_COMM_WORLD);
            break;
        }
    }
}

// void *accept_requests(void *task_queue_arg) {
//    b_queue_t *task_queue = (b_queue_t *) task_queue_arg;

//     while (1) {
//         MPI_Request recv_request;
//         MPI_Status recv_status;
//         int recv_completed = 0;

//         MPI_Irecv(NULL, 0, MPI_INT, MPI_ANY_SOURCE, TASK_REQUEST, MPI_COMM_WORLD,
//             &recv_request);

//         do {
//             MPI_Test(&recv_request, &recv_completed, &recv_status);
//             usleep(SLEEP_TIME);
//         } while (!recv_completed && !work_completed);

//         if (work_completed) {
//             MPI_Request_free(&recv_request);
//             break;
//         }

//         task_t task_to_send;
//         if (queue_get(task_queue, &task_to_send) == NULL) { // empty queue
//             MPI_Send(NULL, 0, mpi_task, recv_status.MPI_SOURCE,
//                 TASK_RESPONSE, MPI_COMM_WORLD);
//             continue;
//         }

//         MPI_Send(&task_to_send, 1, mpi_task, recv_status.MPI_SOURCE,
//             TASK_RESPONSE, MPI_COMM_WORLD);
//     }
// }

int main(int argc, char **argv) {
    int provided_thread_support;
    MPI_Init_thread(NULL, NULL, MPI_THREAD_MULTIPLE, &provided_thread_support);
    if (provided_thread_support != MPI_THREAD_MULTIPLE) {
        fprintf(stderr, "Insufficient level of thread support\n");
        exit(EXIT_FAILURE);
    }

    double start_time = MPI_Wtime();

    int proc_rank, comm_size;
    MPI_Comm_rank(MPI_COMM_WORLD, &proc_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_size);

    MPI_Type_contiguous(sizeof(task_t), MPI_BYTE, &mpi_task);
    MPI_Type_commit(&mpi_task);

    b_queue_t task_queue;
    create_queue(&task_queue, NUM_TASKS);

    // pthread_t responding_thread;
    // int ret_code = pthread_create(&responding_thread, NULL, accept_requests, &task_queue);
    // if (ret_code != 0)
    //     handle_pthread_error("pthread_create", ret_code);

    for (int i = 0; i < NUM_ROUNDS; i++) {
        for (int j = 0; j < NUM_TASKS; j++) {
            queue_put(&task_queue, create_task(proc_rank, comm_size, i));
        }
        execute_tasks(&task_queue);
    }
    work_completed = 1;

    // ret_code = pthread_join(responding_thread, NULL);
    // if (ret_code != 0)
    //     handle_pthread_error("pthread_join", ret_code);

    destroy_queue(&task_queue);

    double end_time = MPI_Wtime();
    printf("Time elapsed: %lf\n", end_time - start_time);

    MPI_Type_free(&mpi_task);
    MPI_Finalize();
}
