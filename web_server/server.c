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
#define BACKLOG 10
#define BUFFER_SIZE 1024

void error(const char *msg) {
    perror(msg);
    exit(0);
}

// AI generated b64 encoder
static const char b64_table[] ="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
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
    // int opcode = hdr[0] & 0x0F;
    int masked = hdr[1] & 0x80;
    size_t payload_len = hdr[1] & 0x7F;

    if (payload_len == 126) {
        unsigned char ext[2];
        recv(sock, ext, 2, 0);
        payload_len = (ext[0] << 8) | ext[1];
    } else if (payload_len == 127) {
        unsigned char ext[8];
        recv(sock, ext, 8, 0);
        // For simplicity, only support <= 2^32
        payload_len = 0;
        for (int i = 0; i < 8; i++) payload_len = (payload_len << 8) | ext[i];
    }

    unsigned char mask[4];
    if (masked) recv(sock, mask, 4, 0);

    if (payload_len > max_len) return -1;

    unsigned char *data = (unsigned char *)out;
    recv(sock, data, payload_len, 0);

    if (masked) {
        for (size_t i = 0; i < payload_len; i++) {
            data[i] ^= mask[i % 4];
        }
    }

    out[payload_len] = '\0';
    return payload_len;
}

void ws_send_frame(int sock, const char *msg) {
    size_t len = strlen(msg);
    unsigned char header[2] = {0x81, 0};
    if (len < 126) {
        header[1] = len;
        send(sock, header, 2, 0);
    }
    send(sock, msg, len, 0);
}


void ws_send_http_response(int sock, char *body) {
    char response[BUFFER_SIZE];
    int body_len = strlen(body) + 15; // Add 15 to account for the dict format

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
    // printf("Full Header: \n%s\n", http_header);
    char *key = NULL;
    char *header_copy = strdup(http_header);
    if (!header_copy) return NULL;

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
    char concat_str[128];
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



int main() {
    struct addrinfo hints, *res;
    struct sockaddr_storage their_addr;
    socklen_t their_addrlen = sizeof(their_addr);
    int server_fd;
    int reuse_addr_flag = 1;
    int *ptr_reuse_addr_flag = &reuse_addr_flag;

    memset(&hints, 0, sizeof hints); // will just copy 0s
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    int addr_info = getaddrinfo(NULL, MYPORT, &hints, &res);
    if (addr_info == -1) {
        error("error getaddrinfo");
    }

    server_fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, ptr_reuse_addr_flag,
               sizeof(reuse_addr_flag));

    int bind_conn = bind(server_fd, res->ai_addr, res->ai_addrlen);
    if (bind_conn == -1) {
        error("Unable to start the server");
    }
    printf("starting server: %d\n", bind_conn);

    listen(server_fd, BACKLOG);


    int client_fd, conn_clients[BACKLOG], fd_count = 1;
    int bytes_recv;
    struct pollfd pfds[BACKLOG + 1]; // +1 bc adding the server to pfds
    char buffer[BUFFER_SIZE];

    pfds[0].fd = server_fd;
    pfds[0].events = POLLIN | POLLOUT;
    bool is_websocket[BACKLOG + 1] = {false};

    while (1) {

        int poll_count = poll(pfds, fd_count, -1);
        if (poll_count == -1) {
            error("Poll error");
            exit(1);
        }

        if (pfds[0].revents & POLLIN) {
            client_fd = accept(server_fd, (struct sockaddr *)&their_addr, &their_addrlen);
            if (client_fd == -1) {
                error("client can't connect");
            }

            if (fd_count >= BACKLOG + 1) {
                printf("Connection to full server attempted\n");
                char *msg = "Server full";
                ws_send_http_response(client_fd, msg);
                close(client_fd);
            } else {
                printf("%d successfully connected to the server\n", client_fd);
                conn_clients[fd_count - 1] = client_fd;
                pfds[fd_count].fd = client_fd;
                pfds[fd_count].events = POLLIN;
                fd_count += 1;

                // recv set here to establish websocket connection
                recv(client_fd, buffer, BUFFER_SIZE, 0);
                const char *ws_sec_key = ws_parse_websocket_http(buffer);
                ws_send_websocket_response(client_fd, ws_sec_key);
		is_websocket[client_fd] = true;
            }
        }

        for (int i = 1; i < fd_count; i++) {
            if (pfds[i].revents & POLLIN) {

		if (!is_websocket[pfds[i].fd]) {
		    const char *client_key = ws_parse_websocket_http(buffer);
		    if (client_key) {
			ws_send_websocket_response(pfds[i].fd, client_key);
		    } else {
			close(pfds[i].fd);
			conn_clients[i] = conn_clients[fd_count - 1];
			pfds[i] = pfds[fd_count - 1]; // Other than pos 0 we don't care about the order
			fd_count--;
			i--;
		    }
		} else {
		    bytes_recv = ws_recv_frame(pfds[i].fd, buffer, BUFFER_SIZE);
		    if (bytes_recv > 0) {
			printf("From %d: %s\n", pfds[i].fd, buffer);

			for (int j = 1; j < fd_count; j++) {
			    if (is_websocket[j] && pfds[j].fd != pfds[i].fd) {
				ws_send_frame(pfds[j].fd, buffer);
			    }
			}
		    }
		}
            }
        }
    }
}
