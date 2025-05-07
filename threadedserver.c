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

#define MYPORT "80"
#define BACKLOG 50 // how many pending connections queue will hold
#define BUFFER_SIZE 1024

void error(const char *msg) {
    perror(msg);
    exit(0);
}

// custom struct to pass values
typedef struct {
    int sock_fd;
} thread_config_t;

char *receive_HTTP_request(int new_connection_fd) {
    int bytes_recv;
    char *ptr_http_request_buffer = malloc(BUFFER_SIZE);
    bytes_recv =
        recv(new_connection_fd, ptr_http_request_buffer, BUFFER_SIZE, 0);
    if (bytes_recv <= 0) {
        error("Error receiving message from client!");
    } else {
        printf("\nBytes received: %d", bytes_recv);
    }

    return ptr_http_request_buffer;
}

char *create_HTTP_response_packet(int malformed) {
    char *ptr_packet_buffer = malloc(BUFFER_SIZE);
    char *ptr_body;
    int body_len;
    if (malformed == 1) {
        ptr_body = "<body>\r\n"
                   "Error 404!\r\n"
                   "</body>\r\n";
        body_len = strlen(ptr_body);
        snprintf(ptr_packet_buffer, BUFFER_SIZE,
                 "HTTP/1.1 400 Bad Request\r\n"
                 "Content-Length: %d\r\n"
                 "Content-Type: text/html;\r\n\r\n"
                 "%s",
                 body_len, ptr_body);
    } else {
        ptr_body = "<body>\r\n"
                   "Hello, Response Packet!\r\n"
                   "</body>\r\n";
        body_len = strlen(ptr_body);
        // format http response, will be stored in packet_buffer
        snprintf(ptr_packet_buffer, BUFFER_SIZE,
                 "HTTP/1.1 200 OK\r\n"
                 "Content-Length: %d\r\n"
                 "Content-Type: text/html;\r\n\r\n"
                 "%s",
                 body_len, ptr_body);
    }

    return ptr_packet_buffer;
}

void send_http_response(int new_connection_fd, char *ptr_packet_buffer) {
    send(new_connection_fd, ptr_packet_buffer, strlen(ptr_packet_buffer), 0);
    free(ptr_packet_buffer);
}

void parse_HTTP_requests(int new_connection_fd) {
    int malformed = 0;
    char *ptr_http_client_buffer = receive_HTTP_request(new_connection_fd);
    char *token = strtok(ptr_http_client_buffer, "\r\n");

    printf("\n");
    printf("HTTP Packet received from browser/client:\n");
    int status_line_found = 0;
    char *ptr_status_line;
    while (token != NULL) {
        printf("%s\n", token);
        if (status_line_found == 0) {
            ptr_status_line = token;
            status_line_found = 1;
        }
        token = strtok(NULL, "\r\n"); // split string by delimitter CRLF
    }
    printf("\n");

    // extract HTTP method and version used from client
    char *ptr_http_method_used = strtok(ptr_status_line, " ");
    char *ptr_http_URI = strtok(NULL, " ");
    char *ptr_http_version_used = strtok(NULL, " ");
    printf("HTTP Method Used (from client packet): %s\n", ptr_http_method_used);
    printf("HTTP URI (from client packet): %s\n", ptr_http_URI);
    printf("HTTP Version Used (from client packet): %s\n\n",
           ptr_http_version_used);

    // TODO: APPLY FUNCTIONALITY... Respond with valid http packet
    //  check for presence of required http stuff
    //  just printing sht now
    if (ptr_http_method_used == NULL) {
        printf("Missing HTTP Method!\n");
        malformed = 1;
    }
    if (ptr_http_URI == NULL) {
        printf("Missing URI!\n");
        malformed = 1;
    }
    if (ptr_http_version_used == NULL) {
        printf("Missing HTTP Version!\n");
        malformed = 1;
    }

    // actually validate the request methods if all http stuff is present
    if (!(strcmp(ptr_http_method_used, "GET") == 0 ||
          strcmp(ptr_http_method_used, "POST") == 0 ||
          strcmp(ptr_http_method_used, "HEAD") == 0)) {
        printf("Invalid http method!\n");
        malformed = 1;
    }
    if (!(ptr_http_URI[0] == '/')) {
        printf("Invalid URI!\n");
        malformed = 1;
    }
    if (!(strcmp(ptr_http_version_used, "HTTP/1.0") == 0 ||
          strcmp(ptr_http_version_used, "HTTP/1.1") == 0)) {
        printf("Unsupported or Invalid HTTP version!\n");
        malformed = 1;
    }

    char *ptr_packet_buffer = create_HTTP_response_packet(malformed);
    send_http_response(new_connection_fd, ptr_packet_buffer);
    free(ptr_http_client_buffer);
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

    parse_HTTP_requests(new_connection_fd);

    // free mem
    close(new_connection_fd);
    free(ptr_client_config);

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
