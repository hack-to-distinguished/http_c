#include <netdb.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#define QUEUE_SIZE 2048
#define NUM_OF_THREADS 8
#define MYPORT "8080"
#define BACKLOG 50
#define BUFFER_SIZE 2048

void error(const char *msg) {
    perror(msg);
    exit(1);
}

void HEADER_NAME_STATE(char **ptr_ptr_http_client_buffer, int new_connection_fd,
                       bool host_header_present, char *ptr_uri,
                       char *ptr_method);
void HEADER_VALUE_STATE(char **ptr_ptr_http_client_buffer,
                        int new_connection_fd, bool host_header_present,
                        char *ptr_uri, char *ptr_method);
void ERROR_STATE(int new_connection_fd);

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

thread_pool_t *thread_pool;

char *receive_HTTP_request(int new_connection_fd) {
    char *ptr_http_request_buffer = malloc(BUFFER_SIZE + 1);

    int total_received = 0;
    int bytes_recv;

    // recv(int fd, void *buf, size_t n, int flags)
    while ((bytes_recv = recv(new_connection_fd, ptr_http_request_buffer,
                              BUFFER_SIZE, 0)) > 0) {
        total_received += bytes_recv;

        if (strstr(ptr_http_request_buffer, "\r\n\r\n")) {
            break;
        }

        if (total_received >= BUFFER_SIZE) {
            perror("request too large");
            free(ptr_http_request_buffer);
            return NULL;
        }
    }

    if (bytes_recv == -1) {
        perror("recv failed");
        free(ptr_http_request_buffer);
        return NULL;
    } else if (bytes_recv == 0 && total_received == 0) {
        perror("client disconnected");
        free(ptr_http_request_buffer);
        return NULL;
    }

    ptr_http_request_buffer[total_received] = '\0';
    return ptr_http_request_buffer;
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
               "Error 400! Bad Request\r\n"
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
                        char *ptr_uri, char *ptr_method) {
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
        if (!(buffer[j] == '\r' || buffer[j] == '\n') && !header_value_found) {
            header_value[counter] = buffer[j];
            counter += 1;
        } else {
            header_value[counter] = '\0';
            header_value_found = true;
        }

        if (j < (strlen(buffer) - 1)) {
            if (buffer[j] == '\r' && buffer[j + 1] == '\n' &&
                !single_crlf_found) {
                single_crlf_found = true;
                *ptr_ptr_http_client_buffer = &buffer[j + 2];
            }
        }
    }

    if (single_crlf_found) {
        // printf("\nHeader Value Extracted: %s\n", header_value);
        HEADER_NAME_STATE(ptr_ptr_http_client_buffer, new_connection_fd,
                          host_header_present, ptr_uri, ptr_method);
        return;
    } else {
        printf("\nerror at header value state");
        ERROR_STATE(new_connection_fd);
        return;
    }
}

size_t get_size_of_file(FILE *fp) {
    fseek(fp, 0, SEEK_END);
    size_t size_of_file = ftell(fp);
    // reset file pointer to point back to beginning of the file
    fseek(fp, 0, SEEK_SET);
    return size_of_file;
}

char *get_file_type_from_uri(char *ptr_uri_buffer) {
    // get file type
    char *file_type = malloc(sizeof(char) * 8);
    int counter = 0;
    bool past_period = false;
    for (int i = 0; i < strlen(ptr_uri_buffer); i++) {

        if (ptr_uri_buffer[i] == '.') {
            past_period = true;
            i += 1;
        }

        if (past_period && ptr_uri_buffer[i] != '\0') {
            file_type[counter] = ptr_uri_buffer[i];
            counter += 1;
        }
    }

    file_type[counter] = '\0';
    return file_type;
}

void send_requested_file_back(int new_connection_fd, char *ptr_uri_buffer) {
    FILE *file_ptr;
    int counter;
    char *file_type = get_file_type_from_uri(ptr_uri_buffer);

    if (strcmp(file_type, "txt") == 0 || strcmp(file_type, "html") == 0) {

        file_ptr = fopen(ptr_uri_buffer, "r");
        size_t size = get_size_of_file(file_ptr);

        char ch;
        char *ptr_file_contents = malloc(sizeof(char) * size);
        char *ptr_packet_buffer = malloc(BUFFER_SIZE + size);
        counter = 0;
        size_t file_contents_len;

        if (file_ptr == NULL) {
            close(new_connection_fd);
            perror("Can't open file.");
            exit(-1);
            return;
        }

        while ((ch = fgetc(file_ptr)) != EOF) {
            ptr_file_contents[counter] = ch;
            counter += 1;
        }

        ptr_file_contents[counter] = '\0';

        // printf("\nsize of file: %ld", size);

        fclose(file_ptr);

        file_contents_len = strlen(ptr_file_contents);

        char HTTP_format[] = "HTTP/1.1 200 OK\r\nContent-Length: "
                             "%ld\r\nContent-Type: %s;\r\n\r\n%s";
        // format http response, will be stored in packet_buffer
        if (strcmp(file_type, "txt") == 0) {
            snprintf(ptr_packet_buffer, BUFFER_SIZE, HTTP_format,
                     file_contents_len, "text/plain", ptr_file_contents);
        } else if (strcmp(file_type, "html") == 0) {
            snprintf(ptr_packet_buffer, BUFFER_SIZE, HTTP_format,
                     file_contents_len, "text/html", ptr_file_contents);
        }

        send_http_response(new_connection_fd, ptr_packet_buffer);
        free(file_type);
        free(ptr_file_contents);
        return;
    } else if (strcmp(file_type, "jpg") == 0 || strcmp(file_type, "png") == 0 ||
               strcmp(file_type, "gif") == 0 ||
               strcmp(file_type, "webp") == 0 ||
               strcmp(file_type, "svg") == 0 || strcmp(file_type, "ico") == 0) {
        file_ptr = fopen(ptr_uri_buffer, "rb");

        if (file_ptr == NULL) {
            fprintf(stderr, "\t Can't open file : %s", ptr_uri_buffer);
            close(new_connection_fd);
            exit(-1);
            return;
        }

        size_t size = get_size_of_file(file_ptr);
        // unsigned char img_file_contents[size];
        unsigned char *ptr_img_file_contents =
            malloc(sizeof(unsigned char) * size);
        size_t bytes_read =
            fread(ptr_img_file_contents, sizeof(unsigned char), size, file_ptr);

        // printf("\nsize of file: %ld\n", size);
        // printf("\nbytes read: %ld\n", bytes_read);
        char HTTP_format[] = "HTTP/1.1 200 OK\r\nContent-Length: "
                             "%ld\r\nContent-Type: %s;\r\n\r\n";
        char *ptr_packet_buffer = malloc(BUFFER_SIZE + size);
        if (strcmp(file_type, "jpg") == 0) {
            snprintf(ptr_packet_buffer, BUFFER_SIZE + size, HTTP_format, size,
                     "image/jpeg");
        } else if (strcmp(file_type, "png") == 0) {
            snprintf(ptr_packet_buffer, BUFFER_SIZE + size, HTTP_format, size,
                     "image/png");
        } else if (strcmp(file_type, "gif") == 0) {
            snprintf(ptr_packet_buffer, BUFFER_SIZE + size, HTTP_format, size,
                     "image/gif");
        } else if (strcmp(file_type, "webp") == 0) {
            snprintf(ptr_packet_buffer, BUFFER_SIZE + size, HTTP_format, size,
                     "image/webp");
        } else if (strcmp(file_type, "svg") == 0) {
            snprintf(ptr_packet_buffer, BUFFER_SIZE + size, HTTP_format, size,
                     "image/svg+xml");
        } else if (strcmp(file_type, "ico") == 0) {
            snprintf(ptr_packet_buffer, BUFFER_SIZE + size, HTTP_format, size,
                     "image/x-icon");
        }

        send_http_response(new_connection_fd, ptr_packet_buffer);
        send(new_connection_fd, ptr_img_file_contents, size, 0);
        free(ptr_img_file_contents);
        free(file_type);
        fclose(file_ptr);
        return;
    }
    return;
}

void END_OF_HEADERS_STATE(int new_connection_fd, char *ptr_uri,
                          char *ptr_method) {

    char *processed_uri_ptr = ptr_uri;
    if (!(strcmp(ptr_uri, "/") == 0)) {
        processed_uri_ptr += 1;
    }
    char uri_buffer[strlen(processed_uri_ptr)];
    strcpy(uri_buffer, processed_uri_ptr);
    // printf("\nURI at end of headers state: %s", uri_buffer);
    char *ptr_uri_buffer = uri_buffer;

    FILE *file_ptr = fopen(uri_buffer, "r");
    size_t len_uri = strlen(uri_buffer);

    struct stat sb;
    stat(uri_buffer, &sb);

    if (access(uri_buffer, F_OK) == 0 && !S_ISDIR(sb.st_mode)) {
        // printf("\nFile exists!");
        send_requested_file_back(new_connection_fd, ptr_uri_buffer);
        free(ptr_uri);
        free(ptr_method);
        fclose(file_ptr);
        return;
    } else if (strcmp(ptr_method, "HEAD") == 0) {
        // printf("\nHEAD Method!");
        if (strcmp(processed_uri_ptr, "") == 0) {
            printf("\nURI is NULL.");
            char *ptr_packet_buffer = malloc(BUFFER_SIZE);
            snprintf(ptr_packet_buffer, BUFFER_SIZE,
                     "HTTP/1.1 200 OK\r\n"
                     "Content-Type: text/html;\r\n\r\n");
            send_http_response(new_connection_fd, ptr_packet_buffer);
            free(ptr_uri);
            free(ptr_method);
            fclose(file_ptr);
            return;
        }
        // } else if (strcmp(ptr_uri)) {
        // }

    } else {
        // printf("\nFile does not exist!");
        char *ptr_packet_buffer = malloc(BUFFER_SIZE);
        char *ptr_body;
        size_t body_len;
        ptr_body = "<body>\r\n"
                   "Error 404! File does not exist\r\n"
                   "</body>\r\n";
        body_len = strlen(ptr_body);
        snprintf(ptr_packet_buffer, BUFFER_SIZE,
                 "HTTP/1.1 404 Not Found\r\n"
                 "Content-Length: %ld\r\n"
                 "Content-Type: text/html;\r\n\r\n"
                 "%s",
                 body_len, ptr_body);
        send_http_response(new_connection_fd, ptr_packet_buffer);
        fclose(file_ptr);
        free(ptr_uri);
        free(ptr_method);
        return;
    }
}

void HEADER_NAME_STATE(char **ptr_ptr_http_client_buffer, int new_connection_fd,
                       bool host_header_present, char *ptr_uri,
                       char *ptr_method) {
    char *buffer = *ptr_ptr_http_client_buffer;
    char header_name[256];
    bool colon_found = false;
    bool single_crlf_found = false;
    int buffer_len = strlen(buffer);
    int counter = 0;

    // printf("\n size of buffer: %d", buffer_len);
    // extract the header name from the header field
    for (int i = 0; i < strlen(buffer); i++) {
        if (buffer[i] == ':') {
            colon_found = true;
            *ptr_ptr_http_client_buffer = &buffer[i];
            i = strlen(buffer);
        }

        if (!single_crlf_found) {
            header_name[counter] = buffer[i];
            counter += 1;
        } else {
            header_name[counter] = '\0';
        }

        if (strlen(buffer) == 2 && buffer[i] == '\r' && buffer[i + 1] == '\n') {
            single_crlf_found = true;
            // printf("\ncrlf found at header name state!");
        }
    }

    int len_header = strlen(header_name);
    // printf("\nlen header: %d", len_header);

    if (single_crlf_found && host_header_present) {
        END_OF_HEADERS_STATE(new_connection_fd, ptr_uri, ptr_method);
        return;
    }

    if (colon_found) {
        // printf("\nHeader Name Extracted: %s", header_name);
        if (strcmp(header_name, "Host") == 0) {
            host_header_present = true;
        }
        HEADER_VALUE_STATE(ptr_ptr_http_client_buffer, new_connection_fd,
                           host_header_present, ptr_uri, ptr_method);
        return;
    } else {
        printf("\nerror at header name state");
        ERROR_STATE(new_connection_fd);
        return;
    }
}

void REQUEST_LINE_STATE(char **ptr_ptr_http_client_buffer,
                        int new_connection_fd) {
    char *buffer =
        *ptr_ptr_http_client_buffer; // dereference the pointer pointer
                                     // to get the actual char buffer
    char *ptr_method = malloc(sizeof(char) * 8);
    char *ptr_uri = malloc(sizeof(char) * 1025);
    char http_version[16];
    bool valid_spacing = false;
    bool host_header_present = false;
    int result = sscanf(buffer, "%s %s %s", ptr_method, ptr_uri, http_version);

    char *crlf_ptr = strstr(buffer, http_version);
    if (crlf_ptr == NULL) {
        ERROR_STATE(new_connection_fd);
        printf("\nerror at request line state");
        free(ptr_method);
        return;
    }
    crlf_ptr += 8;
    if (result != 3) {
        ERROR_STATE(new_connection_fd);
        printf("\nerror at request line state");
        free(ptr_method);
        return;
    }

    if (!(strcmp(ptr_method, "GET") == 0 || strcmp(ptr_method, "POST") == 0 ||
          strcmp(ptr_method, "HEAD") == 0)) {
        ERROR_STATE(new_connection_fd);
        printf("\nerror at request line state");
        free(ptr_method);
        return;
    }

    if (strcmp(http_version, "HTTP/1.1") != 0) {
        ERROR_STATE(new_connection_fd);
        printf("\nerror at request line state");
        free(ptr_method);
        return;
    }

    if (!(crlf_ptr[0] == '\r' && crlf_ptr[1] == '\n')) {
        ERROR_STATE(new_connection_fd);
        printf("\nerror at request line state");
        free(ptr_method);
        return;
    }

    int len_method = strlen(ptr_method);
    int len_uri = strlen(ptr_uri);
    ptr_uri[len_uri] = '\0';

    if (!(buffer[len_method - 1] != ' ' && buffer[len_method] == ' ' &&
          buffer[len_method + 1] == '/' &&
          buffer[len_method + len_uri + 1] == ' ' &&
          buffer[len_method + len_uri + 2] != ' ')) {
        ERROR_STATE(new_connection_fd);
        printf("\nerror at request line state");
        free(ptr_method);
        return;
    }

    crlf_ptr += 2;
    ptr_ptr_http_client_buffer = &crlf_ptr;
    //
    // printf("\nHTTP Method: %s", ptr_method);
    // printf("\nURI: %s", ptr_uri);
    // printf("\nHTTP Version: %s\n", http_version);
    //
    HEADER_NAME_STATE(ptr_ptr_http_client_buffer, new_connection_fd,
                      host_header_present, ptr_uri, ptr_method);
    return;
}

void STATE_PARSER(char *ptr_http_client_buffer, int new_connection_fd) {
    REQUEST_LINE_STATE(&ptr_http_client_buffer, new_connection_fd);
    return;
}

void parse_HTTP_requests(int new_connection_fd) {
    char *ptr_http_client_buffer = receive_HTTP_request(new_connection_fd);
    if (ptr_http_client_buffer == NULL) {
        free(ptr_http_client_buffer);
        return;
    }
    // required as strtok modifies the original ptr data
    // char *dupe_ptr_http_client = malloc(strlen(ptr_http_client_buffer) + 1);
    // memcpy(dupe_ptr_http_client, ptr_http_client_buffer,
    //        strlen(ptr_http_client_buffer) + 1);
    // char *token = strtok(dupe_ptr_http_client, "\r\n");
    //
    // printf("\nHTTP Packet received from browser/client:\n");
    // int status_line_found = 0;
    // char *ptr_status_line;
    // while (token != NULL) {
    //     printf("%s\n", token);
    //     if (status_line_found == 0) {
    //         ptr_status_line = token;
    //         status_line_found = 1;
    //     }
    //     token = strtok(NULL, "\r\n"); // split string by delimitter CRLF
    // }
    // printf("\n");

    STATE_PARSER(ptr_http_client_buffer, new_connection_fd);

    free(ptr_http_client_buffer);
    // free(dupe_ptr_http_client);
    return;
}

// void* generic pointer, allow return of any type of data
// void *args generic input parameters
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

int main(int argc, char *argv[]) {

    thread_pool = malloc(sizeof(thread_pool_t));
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
        }
    }
    freeaddrinfo(res);
}
