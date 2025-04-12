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

void error(const char *msg) {
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
    } else {
        printf("Server name: %s \n", server);
    }

    memset(&hints, 0, sizeof hints); /* will just copy 0s*/
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    int addr_info = getaddrinfo(server, MYPORT, &hints, &res);
    if (addr_info == -1) {
        error("error getaddrinfo");
    }

    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, ptr_reuse_addr_flag,
               sizeof(reuse_addr_flag));

    /*int socket(int domain, int type, int protocol);  */
    sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

    int conn = connect(sockfd, res->ai_addr, res->ai_addrlen);
    printf("starting client connection: %d\n", conn);
    if (conn == -1) {
        error("Unable to start connection");
    }

    char *msg = "SEND SERVER OBESERVER!\n";
    int len_recv, len_sent;
    char buf[1024];
    len_sent = strlen(msg);

    int recv_data = recv(sockfd, buf, len_recv, 0);
    if (recv_data == -1) {
        error("unable to receive data");
    } else {
        printf("Bytes received: %d\n", recv_data);
        printf("Message recieved: %s\n", buf);
    }

    int sent_data = send(sockfd, msg, len_sent, 0);
    if (sent_data == -1) {
        error("Unable to send data");
    } else {
        printf("data sent: %d\n", sent_data);
    }


    // TODO: convert to htonl
    char cmd_msg[1024];
    strcat(cmd_msg, argv[2]);
    strcat(cmd_msg, "\n\r");
    if (send(sockfd, cmd_msg, strlen(cmd_msg), 0) == -1) {
        error("unable to send command line message");
    } else {
        printf("Message sent: s%\n", cmd_msg);
    }

    /*char *cmdl_msg = */
    /*while(1){*/
    /*    if (cmdl_msg != NULL) {*/
    /*char cmd_msg = [128];*/
    /*        if (send(sockfd, cmdl_msg, strlen(cmdl_msg), 0) == -1){*/
    /*            error("unable to send message");*/
    /*        } else {*/
    /*            printf("sent message: %s\n", cmdl_msg);*/
    /*        }*/
    /*    }*/
    /*}*/

    freeaddrinfo(res);
}
