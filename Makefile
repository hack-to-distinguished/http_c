CC = gcc
CFLAGS = -g -O1 -fsanitize=address -Wall -Wextra -pthread

# Threaded HTTP server (main project)
THREADPOOL_SRC = threaded_server_src/http.c threaded_server_src/threadpool.c threaded_server_src/threadpoolserver.c
THREADPOOL_TARGET = threadpoolserver

# WebSocket/HTTP web server (web_server/)
WEBSERVER_DIR = web_server
WEBSERVER_SRC = $(WEBSERVER_DIR)/server.c $(WEBSERVER_DIR)/encode.c $(WEBSERVER_DIR)/decode.c $(WEBSERVER_DIR)/sha1.c
WEBSERVER_TARGET = web_server

# Simple client
CLIENT_SRC = client.c
CLIENT_TARGET = client

# SSH server/client
SSH_SERVER_SRC = ssh_server/server.c
SSH_SERVER_TARGET = ssh_server
SSH_CLIENT_SRC = ssh_server/client.c
SSH_CLIENT_TARGET = ssh_client

.PHONY: all clean

all: $(THREADPOOL_TARGET) $(WEBSERVER_TARGET) $(CLIENT_TARGET) $(SSH_SERVER_TARGET) $(SSH_CLIENT_TARGET)

$(THREADPOOL_TARGET): $(THREADPOOL_SRC)
	$(CC) $(CFLAGS) -o $@ $^

$(WEBSERVER_TARGET): $(WEBSERVER_SRC)
	$(CC) $(CFLAGS) -o $@ $^

$(CLIENT_TARGET): $(CLIENT_SRC)
	$(CC) $(CFLAGS) -o $@ $^

$(SSH_SERVER_TARGET): $(SSH_SERVER_SRC)
	$(CC) $(CFLAGS) -o $@ $^

$(SSH_CLIENT_TARGET): $(SSH_CLIENT_SRC)
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f $(THREADPOOL_TARGET) $(WEBSERVER_TARGET) $(CLIENT_TARGET) $(SSH_SERVER_TARGET) $(SSH_CLIENT_TARGET)
	rm -f threaded_server_src/*.o web_server/*.o ssh_server/*.o
