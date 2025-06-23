#include "threadpool.h"
#include "http.h"
#include <stdio.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

thread_pool_t *thread_pool = NULL;

void *server_thread_to_run(void *args) {
    thread_config_t *ptr_client_config = (thread_config_t *)args;
    // printf("ptr_client_config (thread function): %p\n", ptr_client_config);
    int new_connection_fd = ptr_client_config->sock_fd;

    // using wall-clock time to time how long thread takes to run
    struct timeval start, end;
    double time_used;
    gettimeofday(&start, NULL);

    // int delay_seconds = 1 + rand() % 3; // 1-3 seconds
    // sleep(delay_seconds);

    parse_HTTP_requests(new_connection_fd);

    gettimeofday(&end, NULL);

    time_used =
        (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000.0;
    // printf("\nTime taken: %.4lf seconds to finish thread for fd=%d",
    // time_used,
    //        new_connection_fd);

    // free(ptr_client_config);
    close(new_connection_fd);
    return NULL;
}

void thread_pool_enqueue_t(thread_config_t tct) {
    pthread_mutex_lock(&thread_pool->thread_pool_mutex_t);

    printf("\n[ENQUEUE] FD: %d, Queue size: %zu, Front: %zu, Rear: %zu",
           tct.sock_fd, thread_pool->queue_size, thread_pool->front_pointer,
           thread_pool->rear_pointer);

    if (thread_pool->queue_size < QUEUE_SIZE) {
        thread_pool->queue[thread_pool->rear_pointer] = tct;
        thread_pool->rear_pointer =
            (thread_pool->rear_pointer + 1) % QUEUE_SIZE;
        thread_pool->queue_size += 1;
        // signal/notify worker threads that a task has been added to the thread
        // pool queue
        pthread_cond_signal(&thread_pool->thread_pool_cond_t);
    } else {
        perror("Thread pool queue is full!");
    }
    pthread_mutex_unlock(&thread_pool->thread_pool_mutex_t);
    return;
}

void *worker_thread_t(void *args) {
    while (1) {
        pthread_mutex_lock(&thread_pool->thread_pool_mutex_t);

        printf("\n[WORKER %lu] Waiting for work (Queue size: %zu)",
               pthread_self(), thread_pool->queue_size);

        thread_config_t tct;

        // worker thread sleep until signalled there is a task in queue
        while (thread_pool->queue_size == 0) {
            pthread_cond_wait(&thread_pool->thread_pool_cond_t,
                              &thread_pool->thread_pool_mutex_t);
        }
        if (thread_pool->queue_size > 0) {
            tct = thread_pool->queue[thread_pool->front_pointer];

            printf("\n[WORKER %lu] Processing FD: %d (Queue size: %zu, Front: "
                   "%zu)",
                   pthread_self(), tct.sock_fd, thread_pool->queue_size,
                   thread_pool->front_pointer);

            thread_pool->front_pointer =
                (thread_pool->front_pointer + 1) % QUEUE_SIZE;
            thread_pool->queue_size -= 1;
            pthread_mutex_unlock(&thread_pool->thread_pool_mutex_t);
            server_thread_to_run(&tct);
        } else {
            pthread_mutex_unlock(&thread_pool->thread_pool_mutex_t);
            perror("\nThread pool queue is empty!");
        }
    }
    return NULL;
}

void thread_pool_t_init() {
    thread_pool->front_pointer = 0;
    thread_pool->rear_pointer = 0;
    thread_pool->queue_size = 0;
    pthread_mutex_init(&thread_pool->thread_pool_mutex_t, NULL);
    pthread_cond_init(&thread_pool->thread_pool_cond_t, NULL);
}
