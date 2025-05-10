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
    // Try connecting again if you see that you aren't connected
    // Find a way to check if you're connected, if not, re-establish connection


    // TODO:
    // - Receive the message for the previous user before 
    // sending your message

    char *recv_str = malloc(128);
    int bytes_recv = recv(client_fd, recv_str, 128, 0);
    if (bytes_recv != -1) {
        printf("Messages from the server: %s\n", recv_str);
    }

    puts("Press C-c to quit:");
    char *ptr_str = malloc(128);
    while (1) {
        printf("\nPress enter to send your message: \n");
        scanf("%s", ptr_str);

        int bytes_sent = send(client_fd, ptr_str, strlen(ptr_str), 0);
        if (bytes_sent == -1) {
            error("unable to send entered messaged");
        } else {
            printf("Bytes sent: %d - ", bytes_sent);
            printf("Message sent: %s\n", ptr_str);
        }

        char *recv_str = malloc(128);
        int bytes_recv = recv(client_fd, recv_str, 128, 0);
        if (bytes_recv != -1) {
            printf("Messages from the previous user: %s\n\n", recv_str);
        }

        // INFO: Check that we're still connected - if not reconnect
        /*int num_events = poll(pfds, 1, 2500);*/
        /*if (num_events == 0) {*/
        /*    printf("poll timed out %d\n", num_events);*/
        /*} else { // Reconnect*/
        /*    int pollout_happened = pfds[0].revents & POLLOUT;*/
        /*    if (pollout_happened) {*/
        /*        printf("File descriptor %d is ready to read\n", pfds[0].fd);*/
        /*        connect(client_fd, res->ai_addr, res->ai_addrlen);*/
        /*    } else {*/
        /*        printf("Unexpected event occurred: %d\n", pfds[0].revents);*/
        /*    }*/
        /*}*/


        free(ptr_str);
        ptr_str = malloc(128);
    }
    free(recv_str);


    freeaddrinfo(res);
    return 0;
}
