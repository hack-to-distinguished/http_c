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
#define MAX_CLIENTS 2

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
    int sockfd, new_sockfd;
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

    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, ptr_reuse_addr_flag,
               sizeof(reuse_addr_flag));

    sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

    int bind_conn = bind(sockfd, res->ai_addr, res->ai_addrlen);
    if (bind_conn == -1) {
        error("Unable to start the server");
    }
    printf("starting server: %d\n", bind_conn);

    listen(sockfd, BACKLOG);

    // TODO:
    // [] Send the first connected user back in the queue

    // FIX: NEW ENTRY
    int client_sockfd, conn_clients[MAX_CLIENTS], client_count = 0;
    int bytes_recv, bytes_sent;
    struct pollfd pfds[MAX_CLIENTS];
    while (1) {
        client_sockfd = accept(sockfd, (struct sockaddr *)&their_addr, &their_addrlen);
        if (client_sockfd > 0) {
            if (!is_client_connected(conn_clients, MAX_CLIENTS, client_sockfd)) {
                conn_clients[client_count] = client_sockfd;
                pfds[client_count].fd = client_sockfd;
                pfds[client_count].events = POLLIN | POLLOUT;
                client_count += 1;

                char *msg = "You're connected to the server\n";
                send(client_sockfd, msg, strlen(msg), 0);
            } else {
                char *msg = "Server full\n";
                send(client_sockfd, msg, strlen(msg), 0);
                close(client_sockfd);
            }
        }
        char *ptr_str = malloc(256);
        char *usr_msg_buf = malloc(128);
        for (int i; i < client_count; i++) {
            if (pfds[i].revents & POLLIN) { // Client is sending
                bytes_recv = recv(pfds[i].fd, ptr_str, 512, 0);
                printf("Message received: %s from client %d\n", ptr_str, pfds[i].fd);
                memcpy(usr_msg_buf, ptr_str, bytes_recv);
                fflush(stdout);
            }

            if (pfds[i].revents & POLLOUT) { // Client is ready to recv
                if (usr_msg_buf[0] != '\0' && usr_msg_buf[0] != '\n') {
                    bytes_sent = send(pfds[i].fd, usr_msg_buf, bytes_recv, 0);
                }
                // Will I be stuck here or do I continue?
            }
        }
    }

    // FIX: NEW ENTRY

    /*char *usr_msg_buf = malloc(128);*/
    /*int c1_socket, c2_socket;*/
    /*char *msg = "You're connected to the server,\n";*/
    /*c1_socket = accept(sockfd, (struct sockaddr *)&their_addr, &their_addrlen);*/
    /*send(c1_socket, msg, strlen(msg), 0);*/
    /**/
    /*c2_socket = accept(sockfd, (struct sockaddr *)&their_addr, &their_addrlen);*/
    /*send(c2_socket, msg, strlen(msg), 0);*/
    /**/
    /*struct pollfd pfd[]={{c1_socket, POLLIN, 0}, {c1_socket, POLLIN, 0}};*/
    /**/
    /*int bytes_recv, bytes_sent;*/
    /*while (1) {*/
    /*    char *ptr_str = malloc(256);*/
    /**/
    /*    // INFO: Pretty sure this should just be POLLIN*/
    /*    if (pfd[0].revents & POLLIN || POLLOUT) {*/
    /*        printf("In c1 space\n");*/
    /*        if (usr_msg_buf[0] != '\0' && usr_msg_buf[0] != '\n') {*/
    /*            bytes_sent = send(c1_socket, usr_msg_buf, bytes_recv, 0);*/
    /*            if (bytes_sent != -1) {*/
    /*                printf("Sent message: %s to user %d\n\n", usr_msg_buf, c1_socket);*/
    /*            }*/
    /*        }*/
    /**/
    /*        while ((bytes_recv = recv(c1_socket, ptr_str, 512, 0)) > 0) {*/
    /*            printf("Received: %d bytes from client %d\t", bytes_recv, c2_socket);*/
    /*            printf("Message received: %s\n", ptr_str);*/
    /*            memcpy(usr_msg_buf, ptr_str, bytes_recv);*/
    /*            fflush(stdout);*/
    /*            break;*/
    /*        }*/
    /*    }*/
    /*    if (pfd[1].revents & POLLIN || POLLOUT) {*/
    /*        printf("In c2 space\n");*/
    /*        if (usr_msg_buf[0] != '\0' && usr_msg_buf[0] != '\n') {*/
    /*            bytes_sent = send(c2_socket, usr_msg_buf, bytes_recv, 0);*/
    /*            if (bytes_sent != -1) {*/
    /*                printf("Sent message: %s to user %d\n\n", usr_msg_buf, c2_socket);*/
    /*            }*/
    /*        }*/
    /**/
    /*        while ((bytes_recv = recv(c2_socket, ptr_str, 512, 0)) > 0) {*/
    /*            printf("Received: %d bytes from client %d\t", bytes_recv, c2_socket);*/
    /*            printf("Message received: %s\n", ptr_str);*/
    /*            memcpy(usr_msg_buf, ptr_str, bytes_recv);*/
    /*            fflush(stdout);*/
    /*            break;*/
    /*        }*/
    /*    }*/
    /*}*/
    /*freeaddrinfo(res);*/
}
