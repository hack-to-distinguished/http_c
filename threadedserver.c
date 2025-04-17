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
#define BUFFER_SIZE 1024

// custom struct to pass values
typedef struct {
    int sock_fd;
} thread_config_t;

void send_http_response(int new_connection_fd) {}

void handle_hello(int new_connection_fd) {
    // formatting a HTTP response
    char *ptr_http_hello_response =
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/html; charset=UTF-8\r\n\r\n"
        "<!DOCTYPE html>\r\n"
        "<html>\r\n"
        "<head>\r\n"
        "<title>Testing Basic HTTP-SERVER</title>\r\n"
        "</head>\r\n"
        "<body>\r\n"
        "Hello, Alejandro!\r\n"
        "</body>\r\n"
        "</html>\r\n";
    send(new_connection_fd, ptr_http_hello_response,
         strlen(ptr_http_hello_response), 0);
}

// sending simple byte messages: out of the scope right no was I will be
// focussing on http stuff
void send_simple_byte_messages(int new_connection_fd) {
    int len;
    char *msg = "Alejandro was here!\n";
    len = strlen(msg);

    /*int send(int sockfd, const void *msg, int len, int flags); */
    send(new_connection_fd, msg, len, 0);

    struct sockaddr_in *ptr_peer_addr_in = malloc(sizeof(struct sockaddr_in));
    socklen_t peer_addr_in =
        sizeof(struct sockaddr_in); // socklen_t required as it is basically
                                    // letting the getpeername() function
                                    // know how much data it can safely write to
    char str[peer_addr_in];
    int peer =
        getpeername(new_connection_fd, (struct sockaddr *)ptr_peer_addr_in,
                    (socklen_t *)&peer_addr_in);
    // network to presenetation
    inet_ntop(AF_INET, &(ptr_peer_addr_in->sin_addr), str, peer_addr_in);

    char *second_msg = malloc(128);
    strcat(second_msg, str);
    strcat(second_msg, " is the connected user's IP!\n");
    send(new_connection_fd, second_msg, strlen(second_msg), 0);

    // free memory
    free(second_msg);
    free(ptr_peer_addr_in);
}

void error(const char *msg) {
    perror(msg);
    exit(0);
}

// void* generic pointer, allow return of any type of data
// void *args generic input parameters
void *server_thread_to_run(void *args) {
    thread_config_t *ptr_client_config = (thread_config_t *)args;
    printf("ptr_client_config (thread function): %p\n", ptr_client_config);
    int new_connection_fd = ptr_client_config->sock_fd;

    // using wall-clock time to time how long thread takes to run
    struct timeval start, end;
    double time_used;
    gettimeofday(&start, NULL);

    // here i made it artificially do work by just adding a random time delay so
    // it is actually easier to see the concurrency work in action with the
    // threads
    int delay_seconds = 1 + rand() % 6; // 1-3 seconds
    sleep(delay_seconds);

    handle_hello(new_connection_fd);

    free(ptr_client_config);

    int bytes_recv;
    char buf[BUFFER_SIZE];
    bytes_recv = recv(new_connection_fd, buf, sizeof(buf), 0);
    if (bytes_recv < 0) {
        error("Error receiving message from clien!");
    } else {
        printf("Message received: %s\n", buf);
    }
    close(new_connection_fd);

    gettimeofday(&end, NULL);
    time_used =
        (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000.0;
    printf("Time taken: %.4lf seconds to finish thread for fd=%d \n", time_used,
           new_connection_fd);
    printf("\n");

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
            ptr_client_config->sock_fd = client_fd;
            printf("\nThread started with fd=%d\n", client_fd);
            printf("ptr_client_config (while loop): %p\n", ptr_client_config);

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
