#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#define MYPORT "8080"
#define MAX_IN 2
#define BUFFER_SIZE 1024


void error(const char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <server-address>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const char *server = argv[1];
    struct addrinfo hints, *res;
    int server_fd;
    int reuse_addr_flag = 1;

    printf("Connecting to server: %s\n", server);

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    if (getaddrinfo(server, MYPORT, &hints, &res) != 0) {
        error("getaddrinfo failed");
    }

    server_fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (server_fd == -1) {
        error("socket failed");
    }

    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &reuse_addr_flag,
               sizeof(reuse_addr_flag));

    if (connect(server_fd, res->ai_addr, res->ai_addrlen) == -1) {
        error("connection failed\n");
    } else {
        printf("Connected to the server\n\n");
    }


    struct pollfd pfds[MAX_IN];
    pfds[0].fd = server_fd;
    pfds[0].events = POLLIN;

    pfds[1].fd = STDIN_FILENO;
    pfds[1].events = POLLIN;

    char send_buf[BUFFER_SIZE], recv_buf[BUFFER_SIZE], init_buf[32];
    int bytes_sent, bytes_recv, fd_count = 2;

    bytes_recv = recv(server_fd, init_buf, 32, 0);
    if (bytes_recv != -1) {
        printf("Messages from the server: %s\n", init_buf);
    }

    while (1) {
        int poll_count = poll(pfds, fd_count, -1);
        if (poll_count == -1) {
            error("Poll error");
            exit(1);
        } else {

            if (pfds[0].revents & POLLIN) {
                printf("\n\n");
                bytes_recv = recv(pfds[0].fd, recv_buf, BUFFER_SIZE, 0);
                printf("MESSAGE RECEIVED: %s\n", recv_buf);
            }
            if (pfds[1].revents & POLLIN) {
                printf("\nPress enter to send your message:\n");
                fgets(send_buf, BUFFER_SIZE, stdin);

                bytes_sent = send(pfds[0].fd, send_buf, strlen(send_buf), 0);
                if (bytes_sent > 0) {
                    printf("Message sent: %s\n", send_buf);
                }
            }
        }
    }
}
