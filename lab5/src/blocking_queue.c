#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#include "blocking_queue.h"
#include "utility.h"

void create_queue(b_queue_t *queue, int capacity) {
    queue->task_array = malloc(capacity * sizeof(task_t));
    if (queue->task_array == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    int ret_code = pthread_mutex_init(&queue->mutex, NULL);
    if (ret_code != 0) {
        handle_pthread_error("mutex_init", ret_code);
    }

    queue->capacity = capacity;
    queue->stored = 0;
    queue->tail_index = 0;
}

void destroy_queue(b_queue_t *queue) {
    free(queue->task_array);
    pthread_mutex_destroy(&queue->mutex);
}

void queue_put(b_queue_t *queue, task_t task) {
    queue_block(queue);

    if (queue->stored == queue->capacity) {
        fprintf(stderr, "The queue is full\n");
        exit(EXIT_FAILURE);
    }

    queue->task_array[queue->tail_index] = task;
    queue->stored++;
    queue->tail_index = (queue->tail_index + 1) % queue->capacity;

    queue_unblock(queue);
}

task_t *queue_get(b_queue_t *queue, task_t *task) {
    queue_block(queue);

    if (queue->stored == 0) {
        queue_unblock(queue);
        return NULL;
    }

    int head_index = (queue->tail_index - queue->stored + queue->capacity) % queue->capacity;
    queue->stored--;
    *task = queue->task_array[head_index];

    queue_unblock(queue);
    return task;
}

int queue_is_empty(const b_queue_t *queue) {
    return queue->stored == 0;
}

void queue_block(b_queue_t *queue) {
    int ret_code = pthread_mutex_lock(&queue->mutex);
    if (ret_code != 0) {
        handle_pthread_error("mutex_lock", ret_code);
    }
}

void queue_unblock(b_queue_t *queue) {
    int ret_code = pthread_mutex_unlock(&queue->mutex);
    if (ret_code != 0) {
        handle_pthread_error("mutex_unlock", ret_code);
    }
}
