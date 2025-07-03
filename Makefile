# threadpoolserver: threaded_server_src/http.c threaded_server_src/threadpool.c threaded_server_src/threadpoolserver.c
# 	gcc -o threadpoolserver threaded_server_src/http.c threaded_server_src/threadpool.c threaded_server_src/threadpoolserver.c 
#
# clean:
# 	rm -f threadpoolserver 
#

CC = gcc
CFLAGS = -g -O1 -fsanitize=address -Wall -Wextra -pthread
SRC = threaded_server_src/http.c threaded_server_src/threadpool.c threaded_server_src/threadpoolserver.c
TARGET = threadpoolserver

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET)

