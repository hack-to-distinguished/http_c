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
    // - Accept connections from multiple clients (they don't need to be able
    //  to send messages, just let them connect)
    // - When a message is received just echo it back to all clients
    // - Refine in future PR
    while (1) {
        // TODO: Change this to listen to multiple conn
        // All places that have new_sockfd need to change to be able to listen 
        // to multiple connections
        new_sockfd = accept(sockfd, (struct sockaddr *)&their_addr, &their_addr_len);

        char *msg = "You're connected to the server.\n";
        int len;
        len = strlen(msg);

        send(new_sockfd, msg, len, 0);
        send_http_response(new_sockfd, msg);

        struct sockaddr_in peer_addr_in;
        int peer_addr_in_len = sizeof(peer_addr_in);

        int peer = getpeername(new_sockfd, (struct sockaddr *)&peer_addr_in,
                               (socklen_t *)&peer_addr_in_len);
        char *their_ipv4_addr = inet_ntoa(peer_addr_in.sin_addr);

        char *ip_msg = malloc(128);
        strcat(ip_msg, their_ipv4_addr);
        strcat(ip_msg, " is the IP address of the user!");
        printf("%s\n", ip_msg);
        free(ip_msg);

        // TODO: format the received msg + add an end character so the messages don't get split up

        char *ptr_str;
        int bytes_recv;
        ptr_str = malloc(256);
        // INFO: Once someone enters this loop I can't listen to new conn
        // TODO: Try bringing this into the main loop
        while ((bytes_recv = recv(new_sockfd, ptr_str, 512, 0)) > 0) {
            printf("Received: %d bytes from client %d\t", bytes_recv, new_sockfd);
            printf("Message received: %s\n", ptr_str);
            break;
            fflush(stdout);
        }
        if (bytes_recv == 0) {
            printf("Client %d disconnected.\n", new_sockfd);
        } else if (bytes_recv == -1) {
            error("Error receiving message in while loop");
        }

        // TODO: See if you can add an if disconnect here
        printf("Closing connection for client %d.\n", new_sockfd);
        int closed = close(new_sockfd);
        if (closed == 0) {
            printf("Connection successfully closed. Status: %d.\n", closed);
        }
    }
    freeaddrinfo(res);
}
