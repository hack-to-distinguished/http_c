#include <_stdio.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/_types/_ssize_t.h>

typedef struct {
    char *buffer;
    size_t *bufferLength;

} inputLineBuffer;

inputLineBuffer *createInputLineBuffer() {
    inputLineBuffer *ptrInputLineBuffer = malloc(sizeof(inputLineBuffer));
    ptrInputLineBuffer->bufferLength = 0;
    ptrInputLineBuffer->buffer = NULL;
    return ptrInputLineBuffer;
}

void getLineInput(inputLineBuffer *iPL) {
    char *userInput = NULL;
    // size_t len = 0;
    size_t *len = malloc(sizeof(size_t));
    ssize_t nRead;
    nRead = getline(&userInput, len, stdin);

    if (nRead == -1) {
        fprintf(stderr, "\nError occurred upon getting line from user.");
        return;
    }
    if (nRead == 0) {
        fprintf(stderr, "\nEOF was reach before any characters were read.");
        return;
    }

    iPL->buffer = userInput;
    iPL->bufferLength = len;

    // printf("\nBytes Read: %ld", nRead);
    // printf("\nLen: %ld", len);
    return;
}

void processLineInput() { return; }

int main() {
    inputLineBuffer *iPL = createInputLineBuffer();
    getLineInput(iPL);
    printf("\niPL Buffer: %s", iPL->buffer);
    printf("\niPL Buffer Length: %zu", *iPL->bufferLength);
    // free(iPL->buffer);
    return 0;
}
