#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <poll.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define MYPORT "8080"
#define BACKLOG 10 // how many pending connections queue will hold
#define BUFFER_SIZE 1024

void error(const char *msg) {
    perror(msg);
    exit(0);
}

bool is_client_connected(int arr[], int size, int k) {
    for (int i; i < size; i++) {
        if (arr[i] == k) {
            return true;
        }
    }
    return false;
}

void send_http_response(int sock, const char *body) {
    char response[BUFFER_SIZE];
    printf("sizeof response: %ld\n", sizeof(response));
    int body_len = strlen(body);

    /*
     * response is the pointer to character buffer where the formatted string
     * will be written.
     * \r\n terminates the header
     * %s placeholder for body
     */
    snprintf(response, sizeof(response),
         "HTTP/1.1 200 OK\r\n"
         "Content-Type: text/plain\r\n"
         "Content-Length: %d\r\n"
         "Connection: close\r\n"
         "\r\n"
         "%s",
         body_len, body);
    write(sock, response, strlen(response));
}

int main(int argc, char *argv[]) {
    struct addrinfo hints, *res;
    struct sockaddr_storage their_addr;
    socklen_t their_addrlen = sizeof(their_addr);
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

    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, ptr_reuse_addr_flag,
               sizeof(reuse_addr_flag));

    server_fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

    int bind_conn = bind(server_fd, res->ai_addr, res->ai_addrlen);
    if (bind_conn == -1) {
        error("Unable to start the server");
    }
    printf("starting server: %d\n", bind_conn);

    listen(server_fd, BACKLOG);


    int client_sockfd, conn_clients[BACKLOG], fd_count = 1;
    int bytes_recv, bytes_sent;
    struct pollfd pfds[BACKLOG + 1]; 
    char buffer[BUFFER_SIZE];
    // INFO: +1 bc I'm adding the server to pfds so I can do .revents

    pfds[0].fd = server_fd;
    pfds[0].events = POLLIN;

    while (1) {

        int poll_count = poll(pfds, fd_count, -1);
        if (poll_count == -1) {
            error("Poll error");
            exit(1);
        }

        if (pfds[0].revents & POLLIN) {
            client_sockfd = accept(server_fd, (struct sockaddr *)&their_addr, &their_addrlen);

            if (client_sockfd == -1) {
                error("client can't connect");
            }

            if (!is_client_connected(conn_clients, BACKLOG, client_sockfd)) {
                conn_clients[fd_count] = client_sockfd;
                pfds[fd_count].fd = client_sockfd;
                pfds[fd_count].events = POLLIN | POLLOUT;
                fd_count += 1;

                char *msg = "You're connected to the server\n";
                send(client_sockfd, msg, strlen(msg), 0);
                printf("SENT MSG TO CLIENT\n");
            } else {
                char *msg = "Server full\n";
                send(client_sockfd, msg, strlen(msg), 0);
                close(client_sockfd);
            }
        }

        /*char *ptr_str = malloc(256);*/
        /*char *usr_msg_buf = malloc(128);*/
        for (int i = 1; i < fd_count; i++) {
            if (pfds[i].revents & POLLIN) {
                bytes_recv = recv(pfds[i].fd, buffer, BUFFER_SIZE, 0);
                if (bytes_recv < 0) {
                    printf("Client disconnected\n");
                    close(pfds[i].fd);
                    pfds[i] = pfds[fd_count - 1];
                    // This works bc outside of pos 0 we don't care about the order
                    fd_count--;
                    i--;
                } else {
                    buffer[bytes_recv] = '\0'; // make eof
                    printf("Message received: %s from %d\n", buffer, pfds[i].fd);
                    /*memcpy(usr_msg_buf, buffer, bytes_recv);*/
                    /*fflush(stdout);*/

                    for (int j = 1; j < fd_count; j++) {
                        bytes_sent = send(pfds[j].fd, buffer, bytes_recv, 0);
                    }
                }
            }
        }
    }
}
