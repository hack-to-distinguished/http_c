# Compiler and flags
CC = gcc
CFLAGS = -g -O1 -Wall -Wextra -pthread -fsanitize=address -fno-omit-frame-pointer

# Threadpool HTTP server
THREADPOOL_SRCS = threaded_server_src/http.c threaded_server_src/threadpool.c threaded_server_src/threadpoolserver.c
THREADPOOL_BIN = threadpoolserver

# Web server (WebSocket)
WEBSERVER_SRCS = web_server/server.c web_server/sha1.c
WEBSERVER_BIN = web_server/server

# SSH server/client
SSH_SERVER_SRCS = ssh_server/server.c
SSH_SERVER_BIN = ssh_server/server
SSH_CLIENT_SRCS = ssh_server/client.c
SSH_CLIENT_BIN = ssh_server/client

# Simple client
CLIENT_SRCS = client.c
CLIENT_BIN = client

# Simple Database Management System
SKELETON_SRCS = database/skeleton.c
SKELETON_BIN = database/skeleton

all: $(THREADPOOL_BIN) $(WEBSERVER_BIN) $(SSH_SERVER_BIN) $(SSH_CLIENT_BIN) $(CLIENT_BIN) $(SKELETON_BIN)

$(THREADPOOL_BIN): $(THREADPOOL_SRCS)
	$(CC) $(CFLAGS) -o $@ $^

$(WEBSERVER_BIN): $(WEBSERVER_SRCS)
	$(CC) $(CFLAGS) -o $@ $^

$(SSH_SERVER_BIN): $(SSH_SERVER_SRCS)
	$(CC) $(CFLAGS) -o $@ $^

$(SSH_CLIENT_BIN): $(SSH_CLIENT_SRCS)
	$(CC) $(CFLAGS) -o $@ $^

$(CLIENT_BIN): $(CLIENT_SRCS)
	$(CC) $(CFLAGS) -o $@ $^

$(SKELETON_BIN): $(SKELETON_SRCS)
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f $(THREADPOOL_BIN) $(WEBSERVER_BIN) $(SSH_SERVER_BIN) $(SSH_CLIENT_BIN) $(CLIENT_BIN) $(SKELETON_BIN)

.PHONY: all clean
