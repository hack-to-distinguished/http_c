#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdbool.h>
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

void error(const char *msg) {
    perror(msg);
    exit(1);
}

// prototypes to ensure that there are no undeclared errors and conflicting
// types
void HEADER_NAME_STATE(char **ptr_ptr_http_client_buffer, int new_connection_fd,
                       bool host_header_present, char *ptr_uri);
void HEADER_VALUE_STATE(char **ptr_ptr_http_client_buffer,
                        int new_connection_fd, bool host_header_present,
                        char *ptr_uri);
void ERROR_STATE(int new_connection_fd);

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
        ptr_http_request_buffer[bytes_recv] = '\0';
    }

    return ptr_http_request_buffer;
}

char *create_HTTP_response_packet() {
    char *ptr_packet_buffer = malloc(BUFFER_SIZE);
    char *ptr_body;
    int body_len;
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

    return ptr_packet_buffer;
}

void send_http_response(int new_connection_fd, char *ptr_packet_buffer) {
    send(new_connection_fd, ptr_packet_buffer, strlen(ptr_packet_buffer), 0);
    free(ptr_packet_buffer);
    return;
}

void ERROR_STATE(int new_connection_fd) {
    char *ptr_packet_buffer = malloc(BUFFER_SIZE);
    char *ptr_body;
    int body_len;
    ptr_body = "<body>\r\n"
               "Error 404! File does not exist!\r\n"
               "</body>\r\n";
    body_len = strlen(ptr_body);
    snprintf(ptr_packet_buffer, BUFFER_SIZE,
             "HTTP/1.1 400 Bad Request\r\n"
             "Content-Length: %d\r\n"
             "Content-Type: text/html;\r\n\r\n"
             "%s",
             body_len, ptr_body);
    send_http_response(new_connection_fd, ptr_packet_buffer);
    return;
}

void HEADER_VALUE_STATE(char **ptr_ptr_http_client_buffer,
                        int new_connection_fd, bool host_header_present,
                        char *ptr_uri) {
    bool header_value_found = false;
    bool single_crlf_found = false;
    char *buffer = *ptr_ptr_http_client_buffer;
    char header_value[256];
    int counter = 0;
    int j = 0;

    // this for loop is used to skip redundant characters
    for (int i = 0; i < strlen(buffer); i++) {
        if (buffer[i] == ':' || buffer[i] == ' ' || buffer[i] == '\r' ||
            buffer[i] == '\n') {
            j += 1;
        } else {
            i = strlen(buffer);
        }
    }

    for (j = j; j < strlen(buffer); j++) {
        // getting the header value
        if (!(buffer[j] == '\r' || buffer[j] == '\n') && !header_value_found) {
            header_value[counter] = buffer[j];
            counter += 1;
        } else {
            header_value[counter] = '\0';
            header_value_found = true;
        }

        // check single crlf
        if (j < (strlen(buffer) - 1)) {
            if (buffer[j] == '\r' && buffer[j + 1] == '\n' &&
                !single_crlf_found) {
                single_crlf_found = true;
                *ptr_ptr_http_client_buffer = &buffer[j + 2];
            }
        }
    }

    if (single_crlf_found) {
        printf("\nHeader Value Extracted: %s\n", header_value);
        HEADER_NAME_STATE(ptr_ptr_http_client_buffer, new_connection_fd,
                          host_header_present, ptr_uri);
        return;
    } else {
        printf("\nerror at header value state");
        ERROR_STATE(new_connection_fd);
        return;
    }
}

void END_OF_HEADERS_STATE(int new_connection_fd, char *ptr_uri) {
    // make it so that the pointer skips '/'
    ptr_uri[0] = '\0';
    ptr_uri += 1;
    printf("\nURI at end of headers state: %s", ptr_uri);

    int len_uri = strlen(ptr_uri);
    printf("\nlen of uri value: %d", len_uri);
    for (int i = 0; i < strlen(ptr_uri); i++) {
        printf("\n%c %p", ptr_uri[i], &ptr_uri[i]);
    }
    //
    // check if file exists
    if (access(ptr_uri, F_OK) != -1) {
        printf("\nFile exists!");
        free(ptr_uri - 1);
        char *ptr_packet_buffer = create_HTTP_response_packet();
        send_http_response(new_connection_fd, ptr_packet_buffer);
        return;
    } else {
        printf("\nFile does not exist!");
        ERROR_STATE(new_connection_fd);
        free(ptr_uri - 1);
        return;
    }
    // char *ptr_packet_buffer = create_HTTP_response_packet();
    // send_http_response(new_connection_fd, ptr_packet_buffer);
}

void HEADER_NAME_STATE(char **ptr_ptr_http_client_buffer, int new_connection_fd,
                       bool host_header_present, char *ptr_uri) {
    char *buffer = *ptr_ptr_http_client_buffer;
    char header_name[256];
    bool colon_found = false;
    bool single_crlf_found = false;
    int buffer_len = strlen(buffer);
    int counter = 0;

    printf("\n size of buffer: %d", buffer_len);
    // extract the header name from the header field
    for (int i = 0; i < strlen(buffer); i++) {
        if (buffer[i] == ':') {
            colon_found = true;
            *ptr_ptr_http_client_buffer = &buffer[i];
            i = strlen(buffer);
        }

        // extract the actual header name + put null terminating characters when
        // necessary
        if (!single_crlf_found) {
            header_name[counter] = buffer[i];
            counter += 1;
        } else {
            header_name[counter] = '\0';
        }

        if (strlen(buffer) == 2 && buffer[i] == '\r' && buffer[i + 1] == '\n') {
            single_crlf_found = true;
            printf("\ncrlf found at header name state!");
        }
    }

    int len_header = strlen(header_name);
    printf("\nlen header: %d", len_header);

    if (single_crlf_found && host_header_present) {
        END_OF_HEADERS_STATE(new_connection_fd, ptr_uri);
        return;
    }

    if (colon_found) {
        printf("\nHeader Name Extracted: %s", header_name);
        if (strcmp(header_name, "Host") == 0) {
            host_header_present = true;
        }
        HEADER_VALUE_STATE(ptr_ptr_http_client_buffer, new_connection_fd,
                           host_header_present, ptr_uri);
        return;
    } else {
        printf("\nerror at header name state");
        if (host_header_present) {
            printf("\nhost header present");
        } else {
            printf("\nhost header not present");
        }
        ERROR_STATE(new_connection_fd);
        return;
    }
}

void REQUEST_LINE_STATE(char **ptr_ptr_http_client_buffer,
                        int new_connection_fd) {
    char *buffer =
        *ptr_ptr_http_client_buffer; // dereference the pointer pointer
                                     // to get the actual char buffer
    char method[8];
    char uri[1024];
    char *ptr_uri = malloc(sizeof(char) * 1025);
    char http_version[16];
    bool valid_spacing = false;
    bool host_header_present = false;
    int result = sscanf(buffer, "%s %s %s", method, ptr_uri, http_version);

    char *crlf_ptr = strstr(buffer, http_version);
    if (crlf_ptr == NULL) {
        ERROR_STATE(new_connection_fd);
        printf("\nerror at request line state");
        return;
    }
    crlf_ptr += 8;
    if (result != 3) {
        ERROR_STATE(new_connection_fd);
        printf("\nerror at request line state");
        return;
    }

    if (!(strcmp(method, "GET") == 0 || strcmp(method, "POST") == 0 ||
          strcmp(method, "HEAD") == 0)) {
        ERROR_STATE(new_connection_fd);
        printf("\nerror at request line state");
        return;
    }

    // TODO: check valid uri for file retrieval! later...

    if (strcmp(http_version, "HTTP/1.1") != 0) {
        ERROR_STATE(new_connection_fd);
        printf("\nerror at request line state");
        return;
    }

    if (!(crlf_ptr[0] == '\r' && crlf_ptr[1] == '\n')) {
        ERROR_STATE(new_connection_fd);
        printf("\nerror at request line state");
        return;
    }

    int len_method = strlen(method);
    int len_uri = strlen(ptr_uri);

    if (!(buffer[len_method - 1] != ' ' && buffer[len_method] == ' ' &&
          buffer[len_method + 1] == '/' &&
          buffer[len_method + len_uri + 1] == ' ' &&
          buffer[len_method + len_uri + 2] != ' ')) {
        ERROR_STATE(new_connection_fd);
        printf("\nerror at request line state");
        return;
    }

    crlf_ptr += 2;
    ptr_ptr_http_client_buffer = &crlf_ptr;

    printf("\nHTTP Method: %s", method);
    printf("\nURI: %s", ptr_uri);
    printf("\nHTTP Version: %s\n", http_version);

    HEADER_NAME_STATE(ptr_ptr_http_client_buffer, new_connection_fd,
                      host_header_present, ptr_uri);
    return;
}

void STATE_PARSER(char *ptr_http_client_buffer, int new_connection_fd) {
    REQUEST_LINE_STATE(&ptr_http_client_buffer, new_connection_fd);
    return;
}

void parse_HTTP_requests(int new_connection_fd) {
    char *ptr_http_client_buffer = receive_HTTP_request(new_connection_fd);

    // required as strtok modifies the original ptr data
    char *dupe_ptr_http_client = malloc(strlen(ptr_http_client_buffer) + 1);
    memcpy(dupe_ptr_http_client, ptr_http_client_buffer,
           strlen(ptr_http_client_buffer) + 1);
    char *token = strtok(dupe_ptr_http_client, "\r\n");

    printf("\nHTTP Packet received from browser/client:\n");
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

    STATE_PARSER(ptr_http_client_buffer, new_connection_fd);

    // char *ptr_packet_buffer = create_HTTP_response_packet();
    // send_http_response(new_connection_fd, ptr_packet_buffer);
    free(ptr_http_client_buffer);
    return;
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

    // here i made it artificially do work by just adding a random time
    // delay so it is actually easier to see the concurrency work in action
    // with the threads
    int delay_seconds = 1 + rand() % 3; // 1-3 seconds
    // sleep(delay_seconds);

    parse_HTTP_requests(new_connection_fd);

    // free mem
    close(new_connection_fd);
    free(ptr_client_config);

    gettimeofday(&end, NULL);
    time_used =
        (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000.0;
    printf("\nTime taken: %.4lf seconds to finish thread for fd=%d \n",
           time_used, new_connection_fd);
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
            // malloc data on the heap to avoid race conditions (stop
            // threads accessing shared data)
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
