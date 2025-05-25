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
#define MAX_IN 20


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
    int client_fd;
    int reuse_addr_flag = 1;

    printf("Connecting to server: %s\n", server);

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    if (getaddrinfo(server, MYPORT, &hints, &res) != 0) {
        error("getaddrinfo failed");
    }

    client_fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (client_fd == -1) {
        error("socket failed");
    }

    setsockopt(client_fd, SOL_SOCKET, SO_REUSEADDR, &reuse_addr_flag,
               sizeof(reuse_addr_flag));

    if (connect(client_fd, res->ai_addr, res->ai_addrlen) == -1) {
        error("connection failed");
    } else {
        printf("Connected to the server\n\n");
    }

    // TODO:
    // - Receive the message for the previous user before sending your message
    // -> I have to use the pfds to check that the client is ready to recv msg

    char *recv_str = malloc(128);
    int bytes_recv = recv(client_fd, recv_str, 128, 0);
    if (bytes_recv != -1) {
        printf("Messages from the server: %s\n", recv_str);
    }

    struct pollfd pfd = {client_fd, POLLIN | POLLOUT, 0};
    char *ptr_str = malloc(128);
    int bytes_sent;
    printf("Entering loop\n");
    while (1) {

        // INFO: I think I need to poll a different fd, idk which though
        if (pfd.revents & POLLIN) { // server is sending
            printf("Server sending\n");
            bytes_recv = recv(pfd.fd, recv_str, 128, 0);
            printf("MESSAGE RECEIVED: %s\n", recv_str);
        }
        if (pfd.revents & POLLOUT) {
            printf("\nPress enter to send your message:\n");
            scanf("%s", ptr_str);

            bytes_sent = send(pfd.fd, ptr_str, strlen(ptr_str), 0);
            if (bytes_sent > 0) {
                printf("Message sent: %s\n", ptr_str);
            }
        }

        free(ptr_str);
        ptr_str = malloc(128);
    }
    free(recv_str);


    freeaddrinfo(res);
    return 0;
}
