#include "scanner.h"
#include <stdio.h>
#include <stdlib.h>

void scanTokens(char *buffer) {
    tokenListCTX *ctx = malloc(sizeof(tokenListCTX));
    ctx = initialiseTokenList(8); // just gonna set default size to 8 tokens
    char *startOfLexeme = buffer;
    char *currentPosOfLexeme = startOfLexeme;
    int lineNumber = 1;

    // scan the actual tokens
    while (!isAtEnd(currentPosOfLexeme)) {
        // TODO: how tf to do this
        // start off simple -> assume lexemes will have a length of 1 (just
        // focus on punctuation lexemes)
        startOfLexeme = currentPosOfLexeme;
        currentPosOfLexeme = scanToken(currentPosOfLexeme, ctx);
    }

    printf("\nEnum Token Type: %d", ctx->tail[0].type);

    // printAllTokens(ctx);

    // after scanning the actual tokens, we then add EOF token TODO: will do
    // later
    return;
}

char *scanToken(char *currentPosOfLexeme, tokenListCTX *ctx) {
    char c = *currentPosOfLexeme;
    printf("\nChar c: '%c', ASCII Value: %d", c, c);

    // TODO: Going to just focus on all single lengthed lexemes
    switch (c) {
    case ',':
        addToken(ctx, TOKEN_COMMA);
        break;
    }

    return (currentPosOfLexeme + 1);
};

void addToken(tokenListCTX *ctx, TokenType tokenType) {
    Token *token = malloc(sizeof(Token));
    token->type = tokenType;
    appendToken(*token, ctx);
    return;
};

bool isAtEnd(char *posInBuffer) {
    if (*posInBuffer == '\n' || *posInBuffer == '\0') {
        return true;
    }
    return false;
};
