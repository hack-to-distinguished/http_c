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


void error(const char *msg)
{
    perror(msg);
    exit(0);
}

int main(int argc, char *argv[]) {
    struct addrinfo hints, *res;
    struct sockaddr_storage their_addr;
    socklen_t addr_size;
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

    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, ptr_reuse_addr_flag, sizeof(reuse_addr_flag));

    /*int socket(int domain, int type, int protocol);  */
    sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

    int bind_conn = bind(sockfd, res->ai_addr, res->ai_addrlen); /*int bind(int sockfd, struct sockaddr *my_addr, int addrlen);*/
    printf("starting server: %d\n", bind_conn);


    listen(sockfd, BACKLOG);

    while (1) {
    /*int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen); */
        new_sockfd = accept(sockfd, (struct sockaddr *)&their_addr, &addr_size);

        char *msg = "Chris was here!\n";
        int len, bytes_sent;
        len = strlen(msg);
        char buf[32];
        int bytes_recv;

        /*int send(int sockfd, const void *msg, int len, int flags); */
        send(new_sockfd, msg, len, 0);
        /*bytes_recv = recv(sockfd, buf, len, 0);*/
        close(new_sockfd);
    }
    /*freeaddrinfo(res);*/

}
