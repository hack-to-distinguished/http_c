#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define PORT 8080
#define BUFFER_SIZE 1024

void send_http_response(int sock, const char *body) {
    char response[BUFFER_SIZE];
    int body_len = strlen(body);

    /*
        * snprintf() formats and stores a string in the response buffer.
        * response is the pointer to character buffer where the formatted string will be written.
        * sizeof(response) is the maximum size of the buffer.
        * "HTTP/1.1 200 OK\r\n" is the status line of the HTTP response(protocol version, status code, brief description of status code).
        * \r\n terminates the header
        * %s placeholder for body
        * body_len is the length of the body string.
        * body is the actual content of the response.
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

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address = {
        .sin_family = AF_INET, // IPv4
        .sin_addr.s_addr = INADDR_ANY, // Any address
        .sin_port = htons(PORT) // Port number (8080)
    };
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[BUFFER_SIZE] = {0};
    
    // Create socket file descriptor
    /*
        * server_fd is the file descriptor for the socket that it gets assigned to.
        * socket() creates a socket and returns its file descriptor.
        * AF_INET (Address Family - Internet) specifies IPv4
        * SOCK_STREAM specifies TCP(binary value typically 1).
        * The third argument is usually 0 for default protocol (TCP).
        * If socket creation fails, kernel returns -1.
    */
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
    printf("Server_fd: %d\n", server_fd);
    
    
    // Set socket options
    /*
        * setsockopt() sets the socket options.
        * server_fd is the file descriptor from the socket.
        * SOL_SOCKET (Socket Level) indicates we're setting options at the socket API level.
        * SO_REUSEADDR | SO_REUSEPORT (different options) allows the socket to be bound to an address that is already in use.
        * &opt is the pointer to the option value (1 to enable). allows passing different data types.
        * sizeof(opt) is the size of the option value.
        * If it fails it returns -1.
    */
    if ((setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) < 0) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    
    // Bind socket to port
    /*
        * bind() binds the socket to the address and port specified in the address structure.
        * server_fd is the file descriptor from the socket.
        * struct sockaddr * is a type cast converting address to generic socket address structure.
        * &address gets the memory location of the address structure.
        * sizeof(address) returns size of address structure.
        * If it fails it returns -1.
    */

    // printf("Bind: %d\n", bind(server_fd, (struct sockaddr *)&address, sizeof(address)));

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    
    // Start listening
    /*
        * listen() activates socket for incoming connections by converting into a passive (server) socket.
        * server_fd is the file descriptor from the socket and bound with bind().
        * 3 is the maximum length for the queue pending connections.
        * If it fails it returns -1.
    */
    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    
    // Main server loop
    while(1) {
        printf("\nWaiting for connection...\n");
        
        // Accept incoming connection
        /*
            * accept() accepts an incoming connection from the first in the accept queue.
            * server_fd is the file descriptor from the socket and bound with bind().
            * (struct sockaddr *) is a type cast converting address to generic socket address structure
            * &address gets the memory location of the address structure.
            * (socklen_t*) is a type cast converting addrlen to a pointer to socklen_t.
            * &addrlen is initially the size of the address structure but on return holds actual size of client address.
            * If it fails it returns -1.
        */
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            perror("accept");
            exit(EXIT_FAILURE);
        }
        
        // Read HTTP request
        /*
            * read() copies data from the socket into the buffer.
            * new_socket is the file descriptor for the accepted connection.
            * buffer is the memory location where data will be stored.
            * BUFFER_SIZE is the maximum number of bytes to read.
        */
        read(new_socket, buffer, BUFFER_SIZE);
        printf("Request:\n%s\n", buffer);
        
        // Send HTTP response
        send_http_response(new_socket, "Hello from server\n");
        printf("Response sent\n");
        
        close(new_socket);
    }
    
    return 0;
}