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
        Token *ptr;
        ptr = (Token *)malloc(size * sizeof(Token));
    }
    return ctx;
};
