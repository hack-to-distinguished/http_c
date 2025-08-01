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
#define BACKLOG 10
#define BUFFER_SIZE 1024

void error(const char *msg) {
    perror(msg);
    exit(0);
}

void send_http_response(int sock, char *body) {
    char response[BUFFER_SIZE];
    printf("sizeof response: %ld\n", sizeof(response));

    // TODO: make into json by formatting the body properly
    snprintf(response, sizeof(response),
         "HTTP/1.1 200 OK\r\n"
         "Content-Type: text/plain\r\n"
         "Access-Control-Allow-Origin: *\r\n"
         "Content-Length: %ld\r\n"
         "Connection: close\r\n"
         "\r\n"
         "%s",
         strlen(body), body);
    write(sock, response, strlen(response));
}

int main() {
    struct addrinfo hints, *res;
    struct sockaddr_storage their_addr;
    socklen_t their_addrlen = sizeof(their_addr);
    int server_fd;
    int reuse_addr_flag = 1;
    int *ptr_reuse_addr_flag = &reuse_addr_flag;

    memset(&hints, 0, sizeof hints); // will just copy 0s
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    int addr_info = getaddrinfo(NULL, MYPORT, &hints, &res);
    if (addr_info == -1) {
        error("error getaddrinfo");
    }

    server_fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, ptr_reuse_addr_flag,
               sizeof(reuse_addr_flag));

    int bind_conn = bind(server_fd, res->ai_addr, res->ai_addrlen);
    if (bind_conn == -1) {
        error("Unable to start the server");
    }
    printf("starting server: %d\n", bind_conn);

    listen(server_fd, BACKLOG);


    int client_sockfd, conn_clients[BACKLOG], fd_count = 1;
    int bytes_recv;
    struct pollfd pfds[BACKLOG + 1]; // +1 bc adding the server to pfds
    char buffer[BUFFER_SIZE];

    pfds[0].fd = server_fd;
    pfds[0].events = POLLIN | POLLOUT;

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

            if (fd_count >= BACKLOG + 1) {
                printf("Connection to full server attempted\n");
                char *msg = "Server full\n";
                send_http_response(client_sockfd, msg);
                close(client_sockfd);
            } else {
                printf("%d successfully connected to the server\n", client_sockfd);
                conn_clients[fd_count - 1] = client_sockfd;
                pfds[fd_count].fd = client_sockfd;
                pfds[fd_count].events = POLLIN;
                fd_count += 1;

                char *msg = "Connected to the server\n";
                send_http_response(client_sockfd, msg);
                printf("SENT MSG TO THE CLIENT\n");
            }
        }

        for (int i = 1; i < fd_count; i++) {
            if (pfds[i].revents & POLLIN) {
                bytes_recv = recv(pfds[i].fd, buffer, BUFFER_SIZE, 0);
                if (bytes_recv <= 0) {
                    if (bytes_recv == 0) {
                        printf("User %d disconnected\n", pfds[i].fd);
                    } else {
                        perror("Recv error");
                    }
                    close(pfds[i].fd);
                    conn_clients[i] = conn_clients[fd_count - 1];
                    pfds[i] = pfds[fd_count - 1];
                    // Other than pos 0 we don't care about the order
                    fd_count--;
                    i--;
                } else {
                    buffer[bytes_recv] = '\0'; // make eof
                    printf("Message received: %s from %d\n", buffer, pfds[i].fd);

                    for (int j = 1; j < fd_count; j++) {
                        if (pfds[j].fd != pfds[i].fd) {
                            send_http_response(pfds[j].fd, buffer);
                        }
                    }
                }
            }
        }
    }
}
