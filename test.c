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

// good for development hence why we use 8080
#define PORT "8080"
#define BACKLOG 10

void error(const char *msg) {
    perror(msg);
    exit(0);
}

int main(void) {
    struct addrinfo hints, *res;
    struct sockaddr_storage their_addr;
    socklen_t addr_size;
    int server_fd, new_sockfd;
    int reuse_addr_flag = 1;
    int *ptr_reuse_addr_flag = &reuse_addr_flag;
    struct sockaddr_storage foreign_addr;
    socklen_t foreign_addr_len;

    memset(&hints, 0, sizeof hints); /* will just copy 0s*/
    /*hints.ai_family = AF_UNSPEC;*/
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    int addr_info = getaddrinfo(NULL, PORT, &hints, &res);

    if (addr_info == -1) {
        error("error getaddrinfo");
    }

    // struct sockaddr_in server_addr; // struct holds server address info
    // int server_fd; // file descriptor for the socket for the server

    // AF_INET ipv4 family addr
    // SOCK_STREAM -> tcp
    // 0 defines the protocol we are using, in this case ip
    // creates the socket so we can listen for incoming connection requests
    server_fd = socket(AF_INET, SOCK_STREAM,
                       0); // this returns a an integer value file descripdtor;
                           // used to refer to the socket we have just created

    // setting socket options
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, ptr_reuse_addr_flag,
               sizeof(reuse_addr_flag));

    int bnd = bind(server_fd, res->ai_addr, res->ai_addrlen);

    // listen for connection requests from any interface
    listen(server_fd, BACKLOG);

    // accept incoming connection

    while (1) {
        // accept connection and get new file descriptor which will be used only
        // for the lifecycle of that connection, the original socket fd of the
        // server will stay intact in order to listen for more possible
        // connections in the future

        struct sockaddr_in peer_addr_in;
        int peer_addr_in_len = sizeof(peer_addr_in);
        int *ptr_peer_adr_in_len = &peer_addr_in_len;

        int current_connection_fd = accept(
            server_fd, (struct sockaddr *)&foreign_addr, &foreign_addr_len);

        // get peer name
        int peer =
            getpeername(current_connection_fd, (struct sockaddr *)&peer_addr_in,
                        (socklen_t *)&peer_addr_in_len);
        foreign_addr_len = sizeof(foreign_addr);
        char *msg = inet_ntoa(peer_addr_in.sin_addr);
        int len, bytes_sent;
        len = strlen(msg);

        send(current_connection_fd, msg, len, 0);

        msg = "Hello from Alejandro's Server!";
        len = strlen(msg);
        send(current_connection_fd, msg, len, 0);

        close(current_connection_fd);
    }
}
