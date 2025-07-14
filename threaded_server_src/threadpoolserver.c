#include "http.h"
#include "threadpool.h"
#include <netdb.h>
#include <signal.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define NUM_OF_THREADS 8
#define MYPORT "8080"
#define BACKLOG 100

void termination_handler(int signum) {
    printf("\nTerminating the server gracefully!");
    thread_pool_shutdown_t();
    exit(0);
}

int main() {
    signal(SIGPIPE, SIG_IGN);
    thread_pool = malloc(sizeof(thread_pool_t));
    if (thread_pool == NULL) {
        perror("Failed to allocate memory for thread pool");
        exit(EXIT_FAILURE);
    }
    thread_pool_t_init();
    worker_threads_init(NUM_OF_THREADS);

    struct addrinfo hints, *res;
    int server_fd;
    int reuse_addr_flag = 1;
    int *ptr_reuse_addr_flag = &reuse_addr_flag;

    memset(&hints, 0, sizeof hints); /* will just copy 0s*/
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    int addr_info = getaddrinfo(NULL, MYPORT, &hints, &res);
    if (addr_info == -1) {
        perror("error getaddrerinfo");
    }

    /*int socket(int domain, int type, int protocol);  */
    server_fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, ptr_reuse_addr_flag,
               sizeof(reuse_addr_flag));
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEPORT, ptr_reuse_addr_flag,
               sizeof(reuse_addr_flag));
    int bind_conn = bind(server_fd, res->ai_addr,
                         res->ai_addrlen); /*int bind(int sockfd, struct
                                              sockaddr *my_addr, int addrlen);*/
    printf("\nstarting server: %d", bind_conn);

    listen(server_fd, BACKLOG);

    signal(SIGINT, termination_handler);
    while (1) {
        size_t queue_size_t = thread_pool->queue_size;
        struct sockaddr_storage client_addr;
        socklen_t client_addr_len = sizeof(client_addr);
        int client_fd;

        // printf("\nSize: %zu", thread_pool->queue_size);

        if (queue_size_t < QUEUE_SIZE) {

            client_fd = accept(server_fd, (struct sockaddr *)&client_addr,
                               &client_addr_len);
            // error checking
            if (client_fd < 0) {
                perror("error on accepting connection from client");
            } else {
                thread_config_t *tct = malloc(sizeof(thread_config_t));
                tct->sock_fd = client_fd;
                thread_pool_enqueue_t(*tct);
                free(tct);
            }
        }
    }
    freeaddrinfo(res);
}
