#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#define MYPORT "8080"
#define BACKLOG 50 // how many pending connections queue will hold
void error(const char *msg) {
    perror(msg);
    exit(0);
}

// custom struct to pass values
typedef struct {
    int sockfd;
} thread_config_t;

// void* generic pointer, allow return of any type of data
// void *args generic input parameters
void *server_thread_to_run(void *args) {
    thread_config_t *ptr_client_config = (thread_config_t *)args;
    int client_fd = ptr_client_config->sockfd;

    // using wall-clock time to time how long thread takes to run
    struct timeval start, end;
    double time_used;
    gettimeofday(&start, NULL);

    // here i made it artificially do work by just adding a random time delay so
    // it is actually easier to see the concurrency work in action with the
    // threads
    int delay_seconds = 1 + rand() % 6; // 1-3 seconds
    sleep(delay_seconds);

    int len, bytes_sent;
    int bytes_recv;
    char *msg = "Alejandro was here!\n";
    len = strlen(msg);
    char buf[len];

    /*int send(int sockfd, const void *msg, int len, int flags); */
    send(client_fd, msg, len, 0);

    // TODO: Will work on this later...
    //  struct sockaddr_in peer_addr_in;
    //  int peer_addr_in_len = sizeof(peer_addr_in);
    //  int *ptr_peer_adr_in_len = &peer_addr_in_len;
    //  socklen_t their_addr_len;
    //
    //  int peer = getpeername(client_fd, (struct sockaddr *)&peer_addr_in,
    //                         (socklen_t *)&peer_addr_in_len);
    //  their_addr_len = sizeof(their_addr_len);
    //  char *their_ipv4_addr = inet_ntoa(peer_addr_in.sin_addr);
    //  char *msg = malloc(128);
    //  strcat(msg, their_ipv4_addr);
    //  strcat(msg, " is the IP address of the user!");
    //  strcat(msg, "\n");
    //  len = strlen(msg);
    //  send(client_fd, msg, len, 0);
    //  free(msg);

    // char buf[512];
    // bytes_recv = recv(client_fd, buf, sizeof(buf), 0);
    // if (bytes_recv == -1) {
    //     error("Error receiving message");
    // } else {
    //     // for (int i = 0; i < bytes_recv; i++) {
    //     //     char c = buf[i];
    //     //     if (c == 'f') {
    //     //         printf("letter f present in string\n");
    //     //     }
    //     //     // printf("Char: %s\n", &c);
    //     //     // printf("Char: %s\n", &buf[i]);
    //     // }
    //     // printf("Message received: %s\n", buf);
    // }

    gettimeofday(&end, NULL);
    time_used =
        (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000.0;
    printf("Time taken: %.4lf seconds to finish thread for fd=%d \n", time_used,
           client_fd);
    printf("\n");

    close(client_fd);
    return NULL;
}

int main(int argc, char *argv[]) {

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
        error("error getaddrinfo");
    }

    /*int socket(int domain, int type, int protocol);  */
    server_fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, ptr_reuse_addr_flag,
               sizeof(reuse_addr_flag));
    int bind_conn = bind(server_fd, res->ai_addr,
                         res->ai_addrlen); /*int bind(int sockfd, struct
                                              sockaddr *my_addr, int addrlen);*/
    printf("starting server: %d\n", bind_conn);

    listen(server_fd, BACKLOG);

    while (1) {
        printf("Waiting for a connection!\n");

        // setting up client connection
        socklen_t client_addr_len;
        int client_fd;
        struct sockaddr_storage client_addr;

        client_fd = accept(server_fd, (struct sockaddr *)&client_addr,
                           &client_addr_len);

        // error checking
        if (client_fd < 0) {
            perror("ERROR on accepting connection from client!");
        } else {
            // setting up thread
            pthread_t client_thread;
            // malloc data on the heap to avoid race conditions (stop threads
            // accessing shared data)
            thread_config_t *ptr_client_config =
                malloc(sizeof(thread_config_t));
            ptr_client_config->sockfd = client_fd;
            printf("\nThread started with fd=%d\n", client_fd);

            // create the actual thread (comment both these lines out if you
            // want to convert to sequential server)
            pthread_create(&client_thread, NULL, server_thread_to_run,
                           ptr_client_config);
            pthread_detach(client_thread);

            // comment this line out for sequential server to see the time
            // difference between concurrency with threads and not
            // server_thread_to_run(ptr_client_config);
        }
    }
    freeaddrinfo(res);
}
