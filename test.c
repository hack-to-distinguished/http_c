#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
// good for development hence why we use 8080
#define PORT 8080
#define BACKLOG 10

int main() {
  struct sockaddr_storage foreign_addr;
  struct sockaddr_in server_addr; // struct holds server address info
  socklen_t foreign_addr_size;
  int server_fd; // file descriptor for the socket for the server

  // AF_INET ipv4 family addr
  // SOCK_STREAM -> tcp
  // 0 defines the protocol we are using, in this case ip
  // creates the socket so we can listen for incoming connection requests
  server_fd = socket(AF_INET, SOCK_STREAM,
                     0); // this returns a an integer value file descripdtor;
                         // used to refer to the socket we have just created

  server_addr.sin_family = AF_INET; // ipv4
  server_addr.sin_addr.s_addr =
      INADDR_ANY; // accept any connection from any interface as this is going
                  // to be the server
  server_addr.sin_port =
      htons(PORT); // convert port to network byte order (short)
  // (stuct sockaddrr *) declaring generic socke address pointer
  // sizeof() gets the size of the address structure in bytes
  // used for binding the socket to a specigic port and ip
  bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));

  // listen for connection requests from any interface
  listen(server_fd, BACKLOG);

  // accept incoming connection
  foreign_addr_size = sizeof(foreign_addr);
  // accept connection and get new file descriptor which will be used only for
  // the lifecycle of that connection, the original socket fd of the server will
  // stay intact in order to listen for more possible connections in the future
  int current_connection_fd =
      accept(server_fd, (struct sockaddr *)&foreign_addr, &foreign_addr_size);
}
