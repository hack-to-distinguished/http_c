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
        Token *tokenList;
        Token *indexPosition;
        tokenList = (Token *)malloc(size * sizeof(Token));
        printf("\nCreated Token List at: %p", tokenList);
        if (tokenList == NULL) {
            fprintf(stderr,
                    "\nMemory not allocated successfully for token list!");
            exit(1);
        }
        indexPosition = tokenList;
        ctx->tokenList = tokenList;
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
        ctx->tokenList = (Token *)realloc(
            ctx->tokenList, (int)ctx->currentSize * sizeof(Token));
        if (ctx->tokenList == NULL) {
            fprintf(stderr, "\nMemory not reallocated successfully for tail");
            exit(1);
        }
        ctx->indexPosition = ctx->tokenList + ctx->currentSize - 1;
        *ctx->indexPosition = *token;
    }
    return;
}

void printAllTokens(tokenListCTX *ctx) {
    int counter = 0;
    while ((ctx->currentSize > counter)) {
        printf("\nToken Type: %d, Lexeme: %s (%p), Token Address: %p",
               ctx->tokenList[counter].self->type,
               ctx->tokenList[counter].self->lexeme,
               ctx->tokenList[counter].self->lexeme,
               ctx->tokenList[counter].self);
        counter += 1;
    }
    return;
};

void destroyTokenList(tokenListCTX *ctx) {
    // printf("\nDestroying token list!");
    printf("\n");
    for (size_t i = 0; i < ctx->currentSize; i++) {
        printf("\nDestroying Token at address %p!", ctx->tokenList[i].self);
        if (ctx->tokenList[i].type <= 15) {
            free(ctx->tokenList[i].lexeme);
        }
        free(ctx->tokenList[i].self);
    }
    printf("\nFreeing tokenListCTX at address %p\n", ctx);
    printf("\nFreeing tokenList at address %p\n", ctx->tokenList);
    free(ctx->tokenList);
    free(ctx);
    printf("\n");
    return;
};
