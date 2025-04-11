#include <sys/types.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
/*create a listener function that when running just sends a basic response when pinged*/
#define MYPORT "3490"  // the port users will be connecting to
#define BACKLOG 10     // how many pending connections queue will hold


int main(int argc, char *argv[]) {
    struct addrinfo hints, *res;
    struct sockaddr_storage their_addr;
    socklen_t addr_size;
    int sockfd, new_sockfd;

    memset(&hints, 0, sizeof hints); /* will just copy 0s*/
    /*hints.ai_family = AF_UNSPEC;*/
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    getaddrinfo(NULL, MYPORT, &hints, &res);


    /*int socket(int domain, int type, int protocol);  */
    sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

    /*int bind(int sockfd, struct sockaddr *my_addr, int addrlen);*/
    bind(sockfd, res->ai_addr, res->ai_addrlen);

    listen(sockfd, BACKLOG);

    while (1) {
    /*int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen); */
        new_sockfd = accept(sockfd, (struct sockaddr *)&their_addr, &addr_size);

        char *msg = "Chris was here!";
        int len, bytes_sent;
        len = strlen(msg);
        char buf[32];
        int bytes_recv;

        /*int send(int sockfd, const void *msg, int len, int flags); */
        /*bytes_sent = send(sockfd, msg, len, 0);*/
        /*bytes_recv = recv(sockfd, buf, len, 0);*/
        close(new_sockfd);
    }

}
