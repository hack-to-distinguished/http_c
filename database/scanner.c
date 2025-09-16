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

    printAllTokens(ctx);

    // after scanning the actual tokens, we then add EOF token TODO: will do
    // later
    return;
}

char *scanToken(char *currentPosOfLexeme, tokenListCTX *ctx) {
    char c = *currentPosOfLexeme;
    // printf("\nChar c: '%c', ASCII Value: %d", c, c);

    // TODO: Going to just focus on all single lengthed lexemes
    switch (c) {
    // Skip whitespace
    case ' ':
    case '\r':
    case '\t':
    case '\n':
        break;
    // Punctuation
    case ',':
        addToken(ctx, TOKEN_COMMA);
        break;
    case ';':
        addToken(ctx, TOKEN_SEMICOLON);
        break;
    case '(':
        addToken(ctx, TOKEN_LPAREN);
        break;
    case ')':
        addToken(ctx, TOKEN_RPAREN);
        break;
    case '.':
        addToken(ctx, TOKEN_DOT);
        break;

    // Operators
    case '!':
        if (match(currentPosOfLexeme, '=')) {
            currentPosOfLexeme += 1;
            addToken(ctx, TOKEN_OPERATOR_NEQ);
            break;
        } else {
            fprintf(stderr, "\nUnrecognised Input");
            exit(1);
            break;
        }
    case '=':
        addToken(ctx, TOKEN_OPERATOR_EQ);
        break;
    case '<':
        if (match(currentPosOfLexeme, '=')) {
            currentPosOfLexeme += 1;
            addToken(ctx, TOKEN_OPERATOR_LTE);
        } else {
            addToken(ctx, TOKEN_OPERATOR_LT);
        }
        break;
    case '>':
        if (match(currentPosOfLexeme, '=')) {
            currentPosOfLexeme += 1;
            addToken(ctx, TOKEN_OPERATOR_GTE);
        } else {
            addToken(ctx, TOKEN_OPERATOR_GT);
        }
        break;
    case '+':
        addToken(ctx, TOKEN_OPERATOR_PLUS);
        break;
    case '-':
        addToken(ctx, TOKEN_OPERATOR_MINUS);
        break;
    case '*':
        addToken(ctx, TOKEN_OPERATOR_STAR);
        break;
    case '/':
        addToken(ctx, TOKEN_OPERATOR_SLASH);
        break;
    default:
        fprintf(stderr, "\nUnrecognised Input");
        exit(1);
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

bool match(char *posInBuffer, char expectedChar) {
    posInBuffer += 1;
    if (*posInBuffer == expectedChar) {
        return true;
    }
    return false;
};
