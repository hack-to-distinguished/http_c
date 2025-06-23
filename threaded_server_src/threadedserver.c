#include "http.h"
#include "threadpool.h"
#include <netdb.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>

#define NUM_OF_THREADS 8
#define MYPORT "8080"
#define BACKLOG 50

int main(int argc, char *argv[]) {

    thread_pool = malloc(sizeof(thread_pool_t));
    if (thread_pool == NULL) {
        perror("Failed to allocate memory for thread pool");
        exit(EXIT_FAILURE);
    }
    thread_pool_t_init();
    signal(SIGPIPE,
           SIG_IGN); // deepseek -> used for pipe error: when client disconnects
                     // abruptly while server is trying to write data to socket
                     // -> caused when spamming refresh

    pthread_t worker_threads_t[NUM_OF_THREADS];

    for (int i = 0; i < NUM_OF_THREADS; i++) {
        if (pthread_create(&worker_threads_t[i], NULL, &worker_thread_t,
                           NULL) != 0) {
            perror("\nFailed to create thread.");
        }
    }

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

    while (1) {

        // setting up client connection
        socklen_t client_addr_len;
        int client_fd;
        struct sockaddr_storage client_addr;

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
    freeaddrinfo(res);
}
