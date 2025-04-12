#include <sys/types.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>

#define MYPORT "3490"
#define BACKLOG 10     // how many pending connections queue will hold
#define CONN_TYPE ""

void error(const char *msg)
{
    perror(msg);
    exit(0);
}

int main(int argc, char *argv[]) {
    struct addrinfo hints, *res;
    int sockfd;
    int reuse_addr_flag = 1;
    int *ptr_reuse_addr_flag = &reuse_addr_flag;

    char *server = argv[1];

    if (!server) {
        error("Couldn't get a server address");
    } else { printf("Server name: %s \n", server); }

    memset(&hints, 0, sizeof hints); /* will just copy 0s*/
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;


    int addr_info = getaddrinfo(server, MYPORT, &hints, &res);
    if (addr_info == -1) {
        error("error getaddrinfo");
    }

    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, ptr_reuse_addr_flag, sizeof(reuse_addr_flag));

    /*int socket(int domain, int type, int protocol);  */
    sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

    int conn = connect(sockfd, res->ai_addr, res->ai_addrlen);
    printf("starting client connection: %d\n", conn);
    if (conn == -1) {
        error("Unable to start connection");
    }


    freeaddrinfo(res);
}
