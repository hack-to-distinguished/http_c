#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    char *buffer;
    size_t *bufferLength;
    ssize_t *charactersReadInclEOF;

} inputLineBuffer;

inputLineBuffer *createInputLineBuffer() {
    inputLineBuffer *ptrInputLineBuffer = malloc(sizeof(inputLineBuffer));
    ptrInputLineBuffer->bufferLength = 0;
    ptrInputLineBuffer->buffer = NULL;
    return ptrInputLineBuffer;
}

void destroyInputLineBuffer(inputLineBuffer *iPL) {
    free(iPL->buffer);
    free(iPL->bufferLength);
    free(iPL->charactersReadInclEOF);
    free(iPL);
    return;
}

void getLineInput(inputLineBuffer *iPL) {
    char *userInput = NULL;
    size_t *len = malloc(sizeof(size_t));
    ssize_t *charactersRead = malloc(sizeof(ssize_t));
    printf("db > ");
    *charactersRead = getline(&userInput, len, stdin);

    if (*charactersRead == -1) {
        fprintf(stderr, "\nError occurred upon getting line from user.");
        exit(0);
    }
    if (*charactersRead == 0) {
        fprintf(stderr, "\nEOF was reach before any characters were read.");
        exit(0);
    }

    iPL->buffer = userInput;
    iPL->bufferLength = len;
    iPL->charactersReadInclEOF = charactersRead;

    return;
}

void processLineInput(inputLineBuffer *iPL) {
    if (*iPL->charactersReadInclEOF == 1) {
        fprintf(stderr, "\nEmpty input.");
        exit(0);
    }
    // printf("\niPL Buffer: %s", iPL->buffer);
    // printf("\niPL Buffer Length: %zu", *iPL->bufferLength);
    // printf("\niPL Characters Read (incl EOF): %ld\n\n",
    //        *iPL->charactersReadInclEOF);
    return;
}

int main() {
    while (true) {
        inputLineBuffer *iPL = createInputLineBuffer();
        getLineInput(iPL);

        processLineInput(iPL);
        destroyInputLineBuffer(iPL);
    }
    return 0;
}
