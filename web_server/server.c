#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <poll.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include "sha1.h"

#define MYPORT "8080"
#define BACKLOG 20
#define BUFFER_SIZE 2048
#define MAX_CLIENTS 20

void error(const char *msg) {
    perror(msg);
    exit(1);
}

// AI Generated Base64 encoder
static const char b64_table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
size_t ws_base64_encode(const unsigned char *in, size_t in_len, char *out, size_t out_size) {
    size_t i = 0, o = 0;
    while (i < in_len) {
        unsigned char a3[3] = {0,0,0};
        int j;
        for (j = 0; j < 3 && i < in_len; j++, i++)
            a3[j] = in[i];

        if (o + 4 >= out_size)  // prevent overflow
            return 0;

        out[o++] = b64_table[(a3[0] & 0xfc) >> 2];
        out[o++] = b64_table[((a3[0] & 0x03) << 4) | ((a3[1] & 0xf0) >> 4)];
        out[o++] = (j > 1) ? b64_table[((a3[1] & 0x0f) << 2) | ((a3[2] & 0xc0) >> 6)] : '=';
        out[o++] = (j > 2) ? b64_table[a3[2] & 0x3f] : '=';
    }
    if (o < out_size)
        out[o] = '\0';
    return o;  // returns number of bytes written (not counting null terminator)
}

// AI generated frame decoder
ssize_t ws_recv_frame(int sock, char *out, size_t max_len) {
    unsigned char hdr[2];
    if (recv(sock, hdr, 2, 0) <= 0) return -1;

    // int fin = hdr[0] & 0x80;
    int opcode = hdr[0] & 0x0F;
    int masked = hdr[1] & 0x80;
    size_t payload_len = hdr[1] & 0x7F;

    // Handle close frame
    if (opcode == 8) {
        return -1;
    }

    if (payload_len == 126) {
        unsigned char ext[2];
        if (recv(sock, ext, 2, 0) <= 0) return -1;
        payload_len = (ext[0] << 8) | ext[1];
    } else if (payload_len == 127) {
        unsigned char ext[8];
        if (recv(sock, ext, 8, 0) <= 0) return -1;
        // For simplicity, only support <= 2^32
        payload_len = 0;
        for (int i = 4; i < 8; i++) // Use lower 4 bytes only
            payload_len = (payload_len << 8) | ext[i];
    }

    if (payload_len == 0) return 0;
    if (payload_len > max_len - 1) { // Leave room for null terminator
        printf("Message too large (payload_len=%zu, max=%zu)\n", payload_len, max_len - 1);
        return -1;
    }

    unsigned char mask[4] = {0};
    if (masked) {
        if (recv(sock, mask, 4, 0) <= 0) return -1;
    }

    size_t bytes_read = 0;
    while (bytes_read < payload_len) {
        ssize_t result = recv(sock, out + bytes_read, payload_len - bytes_read, 0);
        if (result <= 0) return -1;
        bytes_read += result;
    }

    if (masked) {
        for (size_t i = 0; i < payload_len; i++) {
            out[i] ^= mask[i % 4];
        }
    }

    out[payload_len] = '\0';
    return payload_len;
}

// WebSocket frame sender
void ws_send_frame(int sock, const char *msg) {
    size_t len = strlen(msg);

    printf("Sending to client %d: %s\n", sock, msg);

    // Prepare header (assuming len < 65536)
    unsigned char header[10] = {0};
    header[0] = 0x81; // Text frame, FIN bit set

    if (len < 126) {
        header[1] = len;
        if (send(sock, header, 2, 0) < 2) {
            perror("Failed to send WebSocket header");
            return;
        }
    } else if (len < 65536) {
        header[1] = 126;
        header[2] = (len >> 8) & 0xFF;
        header[3] = len & 0xFF;
        if (send(sock, header, 4, 0) < 4) {
            perror("Failed to send WebSocket header");
            return;
        }
    } else {
        header[1] = 127;
        // Set length bytes, assuming len is less than 2^32
        for (int i = 0; i < 8; i++) {
            header[9-i] = (len >> (i * 8)) & 0xFF;
        }
        if (send(sock, header, 10, 0) < 10) {
            perror("Failed to send WebSocket header");
            return;
        }
    }

    // Send payload
    if (send(sock, msg, len, 0) < (ssize_t)len) {
        perror("Failed to send WebSocket payload");
    }
}

void ws_send_http_response(int sock, const char *body) {
    char response[BUFFER_SIZE];
    int body_len = strlen(body) + 15; // Add 15 to account for the JSON format

    snprintf(response, sizeof(response),
         "HTTP/1.1 200 OK\r\n"
         "Content-Type: application/json\r\n"
         "Access-Control-Allow-Origin: *\r\n"
         "Content-Length: %d\r\n"
         "Connection: keep-alive\r\n"
         "\r\n"
         "{\"message\": \"%s\"}",
         body_len, body);
    write(sock, response, strlen(response));
}

void ws_send_websocket_response(int sock, const char *accept_key) {
    char response[BUFFER_SIZE];
    snprintf(response, sizeof(response),
         "HTTP/1.1 101 Switching Protocols\r\n"
         "Upgrade: websocket\r\n"
         "Connection: Upgrade\r\n"
         "Sec-WebSocket-Accept: %s\r\n\r\n",
         accept_key);
    write(sock, response, strlen(response));
}

const char *ws_parse_websocket_http(const char *http_header) {
    if (!http_header || http_header[0] == '\0') {
        printf("Empty HTTP header\n");
        return NULL;
    }

    // Check if it's a WebSocket upgrade request
    if (strstr(http_header, "Upgrade: websocket") == NULL) {
        printf("Not a WebSocket upgrade request\n");
        return NULL;
    }

    char *key = NULL;
    char *header_copy = strdup(http_header);
    if (!header_copy) {
        perror("Failed to allocate memory for header copy");
        return NULL;
    }

    const char *needle = "Sec-WebSocket-Key:";
    char *line = strtok(header_copy, "\r\n");
    while (line != NULL) {
        if (strncmp(line, needle, strlen(needle)) == 0) {
            const char *value_start = line + strlen(needle);
            while (*value_start == ' ') value_start++;
            key = strdup(value_start);
            break;
        }
        line = strtok(NULL, "\r\n");
    }
    free(header_copy);

    if (key == NULL) {
        printf("Error: Sec-WebSocket-Key not found.\n");
        return NULL;
    }

    const char *magic_str = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
    char concat_str[256];
    snprintf(concat_str, sizeof(concat_str), "%s%s", key, magic_str);
    free(key);

    unsigned char digest[20];
    SHA1_CTX ctx;
    SHA1Init(&ctx);
    SHA1Update(&ctx, (const unsigned char*)concat_str, strlen(concat_str));
    SHA1Final(digest, &ctx);

    static char accept_key[32]; // Base64 of 20 bytes is 28 chars + padding + null
    ws_base64_encode(digest, 20, accept_key, sizeof(accept_key));
    printf("Sec-WebSocket-Accept: %s\n", accept_key);
    return accept_key;
}

// Client data structure
typedef struct {
    int fd;
    bool is_websocket;
    char ip[INET6_ADDRSTRLEN];
} client_t;

int main(int argc, char *argv[]) {
    struct addrinfo hints, *res;
    struct sockaddr_storage their_addr;
    socklen_t their_addrlen = sizeof(their_addr);
    int server_fd;
    int reuse_addr_flag = 1;

    // Initialize client array
    client_t clients[MAX_CLIENTS];
    for (int i = 0; i < MAX_CLIENTS; i++) {
        clients[i].fd = -1;
        clients[i].is_websocket = false;
    }

    memset(&hints, 0, sizeof hints); // will just copy 0s
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    const char *host = (argc > 1) ? argv[1] : NULL;

    int addr_info = getaddrinfo(host, MYPORT, &hints, &res);
    if (addr_info != 0) {
        fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(addr_info));
        exit(1);
    }

    server_fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (server_fd == -1) {
        error("Failed to create socket");
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &reuse_addr_flag, sizeof(reuse_addr_flag)) == -1) {
        error("Failed to set socket options");
    }

    int bind_conn = bind(server_fd, res->ai_addr, res->ai_addrlen);
    if (bind_conn == -1) {
        error("Unable to bind to address");
    }

    freeaddrinfo(res); // No longer needed

    printf("Starting WebSocket server on port %s\n", MYPORT);

    if (listen(server_fd, BACKLOG) == -1) {
        error("Failed to listen");
    }

    struct pollfd pfds[MAX_CLIENTS + 1]; // +1 for the server socket
    pfds[0].fd = server_fd;
    pfds[0].events = POLLIN;
    int fd_count = 1;

    char buffer[BUFFER_SIZE];

    while (1) {
        int poll_count = poll(pfds, fd_count, -1);
        if (poll_count == -1) {
            error("Poll error");
        }

        // Check for new connections
        if (pfds[0].revents & POLLIN) {
            struct sockaddr_in *client_addr;
            int client_fd = accept(server_fd, (struct sockaddr *)&their_addr, &their_addrlen);
            if (client_fd == -1) {
                perror("Accept failed");
                continue;
            }

            client_addr = (struct sockaddr_in*)&their_addr;
            char client_ip[INET6_ADDRSTRLEN];
            inet_ntop(their_addr.ss_family, &client_addr->sin_addr, client_ip, sizeof(client_ip));

            printf("New connection from %s on socket %d\n", client_ip, client_fd);

            // Check if we can accept more clients
            if (fd_count >= MAX_CLIENTS + 1) {
                printf("Server full, rejecting connection from %s\n", client_ip);
                const char *msg = "Server full";
                ws_send_http_response(client_fd, msg);
                close(client_fd);
                continue;
            }

            // Read the initial HTTP request
            memset(buffer, 0, BUFFER_SIZE);
            int bytes_read = recv(client_fd, buffer, BUFFER_SIZE - 1, 0);
            if (bytes_read <= 0) {
                perror("Receive failed");
                close(client_fd);
                continue;
            }
            buffer[bytes_read] = '\0';

            const char *accept_key = ws_parse_websocket_http(buffer);
            if (accept_key) {
                ws_send_websocket_response(client_fd, accept_key);

                for (int i = 0; i < MAX_CLIENTS; i++) {
                    if (clients[i].fd == -1) {
                        clients[i].fd = client_fd;
                        clients[i].is_websocket = true;
                        strncpy(clients[i].ip, client_ip, sizeof(clients[i].ip));
                        break;
                    }
                }

                pfds[fd_count].fd = client_fd;
                pfds[fd_count].events = POLLIN;
                fd_count++;

                printf("WebSocket connection established with %s on socket %d\n", client_ip, client_fd);
            } else {
                printf("Non-WebSocket request from %s, sending HTTP response\n", client_ip);
                ws_send_http_response(client_fd, "This server only accepts WebSocket connections");
                close(client_fd);
                continue;
            }
        }

        // Check existing connections for data
        for (int i = 1; i < fd_count; i++) {
            if (pfds[i].revents & POLLIN) {
                int client_sock = pfds[i].fd;

                int client_idx = -1;
                for (int j = 0; j < MAX_CLIENTS; j++) {
                    if (clients[j].fd == client_sock) {
                        client_idx = j;
                        break;
                    }
                }

                if (client_idx == -1 || !clients[client_idx].is_websocket) {
                    printf("Invalid client socket %d, closing\n", client_sock);
                    close(client_sock);
                    pfds[i] = pfds[fd_count - 1];
                    fd_count--;
                    i--;
                    continue;
                }

                memset(buffer, 0, BUFFER_SIZE);
                ssize_t bytes_recv = ws_recv_frame(client_sock, buffer, BUFFER_SIZE - 1);

                if (bytes_recv <= 0) {
                    printf("Client %s on socket %d disconnected\n", clients[client_idx].ip, client_sock);
                    close(client_sock);

                    clients[client_idx].fd = -1;
                    clients[client_idx].is_websocket = false;

                    pfds[i] = pfds[fd_count - 1];
                    fd_count--;
                    i--;
                    continue;
                }

                printf("Received from %s (%d): %s\n", clients[client_idx].ip, client_sock, buffer);

                for (int j = 0; j < MAX_CLIENTS; j++) {
                    if (clients[j].fd != -1 && clients[j].is_websocket && clients[j].fd != client_sock) {
                        ws_send_frame(clients[j].fd, buffer);
                    }
                }
            }

            // Check for errors or disconnects
            if (pfds[i].revents & (POLLERR | POLLHUP | POLLNVAL)) {
                int client_sock = pfds[i].fd;

                for (int j = 0; j < MAX_CLIENTS; j++) {
                    if (clients[j].fd == client_sock) {
                        printf("Client %s on socket %d has error or disconnected\n", clients[j].ip, client_sock);
                        close(client_sock);
                        clients[j].fd = -1;
                        clients[j].is_websocket = false;
                        break;
                    }
                }

                pfds[i] = pfds[fd_count - 1];
                fd_count--;
                i--;
            }
        }
    }

    close(server_fd);
    return 0;
}
