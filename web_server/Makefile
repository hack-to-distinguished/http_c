CC = gcc
CFLAGS = -g -O1 -Wall -Wextra -pthread
SRC = server.c sha1.c
TARGET = server

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET)

.PHONY: all clean
