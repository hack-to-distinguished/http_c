CC = gcc
CFLAGS = -Wall -Wextra -O2
LDFLAGS =
SRC = server.c encode.c decode.c sha1.c
OBJ = $(SRC:.c=.o)
TARGET = server

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Header dependencies (optional, for clarity)
server.o: server.c sha1.h b64.h
encode.o: encode.c b64.h
decode.o: decode.c b64.h
sha1.o: sha1.c sha1.h

clean:
	rm -f $(OBJ) $(TARGET)
