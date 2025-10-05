#include "token_list.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

tokenListCTX *initialiseTokenList(size_t size) {
    tokenListCTX *ctx = malloc(sizeof(tokenListCTX));
    if (ctx == NULL) {
        fprintf(stderr, "\nMemory not allocated successfully for tokenListCTX");
        exit(1);
    } else {
        Token *tail;
        Token *indexPosition;
        tail = (Token *)malloc(size * sizeof(Token));
        if (tail == NULL) {
            fprintf(stderr, "\nMemory not allocated successfully for tail");
            exit(1);
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

void appendToken(Token *token, tokenListCTX *ctx) {
    if (!isFull(ctx)) {
        *ctx->indexPosition = *token;
        ctx->indexPosition += 1;
        ctx->currentSize += 1;
    } else {
        ctx->currentSize += 1;
        ctx->tail =
            (Token *)realloc(ctx->tail, (int)ctx->currentSize * sizeof(Token));
        if (ctx->tail == NULL) {
            fprintf(stderr, "\nMemory not reallocated successfully for tail");
            exit(1);
        }
        ctx->indexPosition = ctx->tail + ctx->currentSize - 1;
        *ctx->indexPosition = *token;
    }
    return;
}

void printAllTokens(tokenListCTX *ctx) {
    int counter = 0;
    while ((ctx->currentSize > counter)) {
        printf("\nToken Type: %d, Lexeme: %s (%p), Token Address: %p",
               ctx->tail[counter].type, ctx->tail[counter].lexeme,
               &ctx->tail[counter], ctx->tail[counter].self);
        counter += 1;
    }
    return;
};

void destroyTokenList(tokenListCTX *ctx) {
    // printf("\nDestroying token list!");
    printf("\n");
    for (size_t i = 0; i < ctx->currentSize; i++) {
        printf("\nDestroying Token at address %p!", ctx->tail[i].self);
        if (ctx->tail[i].type <= 15) {
            free(ctx->tail[i].lexeme);
        }
        free(ctx->tail[i].self);
    }
    free(ctx->tail);
    printf("\nFreeing tokenListCTX at address %p\n", ctx);
    free(ctx);
    printf("\n");
    return;
};
