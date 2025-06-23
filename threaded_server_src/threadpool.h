#include <pthread.h>
#define QUEUE_SIZE 256

typedef struct {
    int sock_fd;
} thread_config_t;

typedef struct {
    thread_config_t queue[QUEUE_SIZE];
    size_t front_pointer;
    size_t rear_pointer;
    size_t queue_size;
    pthread_mutex_t thread_pool_mutex_t;
    pthread_cond_t thread_pool_cond_t;
} thread_pool_t;

extern thread_pool_t *thread_pool;

void *server_thread_to_run(void *args);
void thread_pool_enqueue_t(thread_config_t tct);
void *worker_thread_t(void *args);
void thread_pool_t_init();
