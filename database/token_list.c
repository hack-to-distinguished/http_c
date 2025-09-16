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
        ctx->currentSize = 0;
    }
    return ctx;
};

bool isFull(tokenListCTX *ctx) { return ctx->currentSize >= ctx->maxSize; }

void appendToken(Token token, tokenListCTX *ctx) {
    if (!isFull(ctx)) {
        *ctx->indexPosition = token;
        ctx->indexPosition += 1;
        ctx->currentSize += 1;
    } else {
        printf("\nReallocating Memory!");
        ctx->currentSize += 1;
        ctx->tail =
            (Token *)realloc(ctx->tail, (int)ctx->currentSize * sizeof(Token));
        if (ctx->tail == NULL) {
            fprintf(stderr, "\nMemory not reallocated successfully for tail");
            exit(0);
        }
        ctx->indexPosition = ctx->tail + ctx->currentSize - 1;
        *ctx->indexPosition = token;
    }
    return;
}

void printAllTokens(tokenListCTX *ctx) {
    for (int i = 0; i < 1; i++) {
        printf("\nToken Type: %d", ctx->tail[i].type);
    }
    return;
};
