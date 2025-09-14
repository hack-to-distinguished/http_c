#include "token_list.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

tokenListCTX *initialiseTokenList(size_t size) {
    tokenListCTX *ctx = malloc(sizeof(tokenListCTX));
    if (ctx == NULL) {
        fprintf(stderr, "\nMemory not allocated successfully for tokenListCTX");
        exit(0);
    } else {
        Token *tail;
        Token *indexPosition;
        tail = (Token *)malloc(size * sizeof(Token));
        if (tail == NULL) {
            fprintf(stderr, "\nMemory not allocated successfully for tail");
            exit(0);
        }
        ctx->tail = tail;
        indexPosition = tail;
        ctx->indexPosition = indexPosition;
        ctx->maxSize = size;
    }
    return ctx;
};

void appendToken(Token token) { return; }
