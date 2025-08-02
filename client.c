#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#define MYPORT "8080"

void error(const char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <server-address>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const char *string_to_parse = argv[2];
    printf("String Inputted: %s\n", string_to_parse);

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
        error("connect failed");
    }

    // send data to server
    int sent_data =
        send(client_fd, string_to_parse, strlen(string_to_parse), 0);
    if (sent_data == -1) {
        error("send failed");
    }

    printf("Message sent to server: %s\n", string_to_parse);

    // recevie data from server
    char buf[1024];
    int bytes_recv;
    while ((bytes_recv = recv(client_fd, buf, sizeof(buf), 0)) > 0) {
        printf("\nMessage(s) recevied from server:\n");
        printf("%s", buf);
    }

    freeaddrinfo(res);
    return 0;
}
