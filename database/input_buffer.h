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

inputLineBuffer *createInputLineBuffer();
void destroyInputLineBuffer(inputLineBuffer *iPL);
void getLineInput(inputLineBuffer *iPL);
void processLineInput(inputLineBuffer *iPL);
