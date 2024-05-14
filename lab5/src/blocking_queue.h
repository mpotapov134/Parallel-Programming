#ifndef BLOCKING_QUEUE_DEFINED
#define BLOCKING_QUEUE_DEFINED

#include <pthread.h>

#include "task.h"

/*
 * Blocking queue of tasks.
 * For simplicity the queue is implemented as a cyclic buffer of fixed size and cannot
 * be resized.
 */

typedef struct {
    task_t *task_array;
    int capacity;
    int stored;
    int tail_index;
    pthread_mutex_t mutex;
} b_queue_t;

void create_queue(b_queue_t *queue, int capacity);

void destroy_queue(b_queue_t *queue);

void queue_put(b_queue_t *queue, task_t task);

task_t *queue_get(b_queue_t *queue, task_t *task);

int queue_is_empty(const b_queue_t *queue);

void queue_block(b_queue_t *queue);

void queue_unblock(b_queue_t *queue);

#endif
