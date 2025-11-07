#include "parser.h"
#include <stddef.h>
#include <stdio.h>

void parse(tokenListCTX *tokenListCTX) {
    for (size_t i = 0; i < tokenListCTX->currentSize; i++) {
        // printf("\n%p", tokenListCTX->head[i].self);
        // match(&tokenListCTX->head[i]);
    }
};

bool match(Token *token) {
    // printf("\nToken Type (match call): %d", token->type);
    return true;
};
