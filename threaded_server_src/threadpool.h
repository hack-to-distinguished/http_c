#include <pthread.h>
#include <stddef.h>
#define QUEUE_SIZE 128

typedef struct {
    int sock_fd;
} thread_config_t;

typedef struct {
    thread_config_t queue[QUEUE_SIZE];
    pthread_t *WORKER_THREADS;
    size_t MAX_WORKERS;
    size_t front_pointer;
    size_t rear_pointer;
    size_t queue_size;
    pthread_mutex_t thread_pool_mutex_t;
    pthread_cond_t thread_pool_cond_t;
    volatile int shutdown;
} thread_pool_t;

extern thread_pool_t *thread_pool;

void *server_thread_to_run(void *args);
void thread_pool_enqueue_t(thread_config_t tct);
void *worker_thread_t(void *args);
void thread_pool_t_init();
pthread_t *worker_threads_init(size_t num_of_workers);
void thread_pool_shutdown_t();
