#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
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
    socklen_t their_addr_len = sizeof(their_addr);
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

    char *usr_msg_buf = malloc(128);
    while (1) {
        int bytes_recv;
        char *ptr_str = malloc(256);

        new_sockfd = accept(sockfd, (struct sockaddr *)&their_addr, &their_addr_len);
        char *msg = "You're connected to the server,\n";
        send(new_sockfd, msg, strlen(msg), 0);

        // INFO: Send the stored message to the next user
        if (usr_msg_buf[0] != '\0' && usr_msg_buf[0] != '\n') {
            int s_status = send(new_sockfd, usr_msg_buf, bytes_recv, 0);
            if (s_status != -1) {
                printf("Sent message: %s to user %d\n\n", usr_msg_buf, new_sockfd);
                free(usr_msg_buf); // FIX: Only works for one message
            }
        }

        while ((bytes_recv = recv(new_sockfd, ptr_str, 512, 0)) > 0) {
            printf("Received: %d bytes from client %d\t", bytes_recv, new_sockfd);
            printf("Message received: %s\n", ptr_str);
            memcpy(usr_msg_buf, ptr_str, bytes_recv);
            fflush(stdout);
            break;
        }

        if (bytes_recv == 0) {
            printf("Client %d disconnected.\n", new_sockfd);
        } else if (bytes_recv == -1) {
            printf("Error receiving message - Client likely disconnected");
        }

    }
    freeaddrinfo(res);
}
