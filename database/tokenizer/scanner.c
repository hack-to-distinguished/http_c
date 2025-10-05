#include "scanner.h"
#include <ctype.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void scanTokens(char *buffer) {
    tokenListCTX *ctx = initialiseTokenList(8);
    char *startOfLexeme = buffer;
    char *currentPosOfLexeme = startOfLexeme;
    int lineNumber = 1;

    // scan the actual tokens
    while (!isAtEnd(currentPosOfLexeme)) {
        startOfLexeme = currentPosOfLexeme;
        currentPosOfLexeme = scanToken(currentPosOfLexeme, ctx, startOfLexeme);
    }
    printAllTokens(ctx);
    destroyTokenList(ctx);
    return;
}

char *scanToken(char *currentPosOfLexeme, tokenListCTX *ctx,
                char *bufferStart) {
    char c = *currentPosOfLexeme;

    switch (c) {
        // Skip whitespace
    case ' ':
    case '\r':
    case '\t':
    case '\n':
        break;
    // Punctuation
    case ',':
        addToken(ctx, TOKEN_COMMA, ",");
        break;
    case ';':
        addToken(ctx, TOKEN_SEMICOLON, ";");
        break;
    case '(':
        addToken(ctx, TOKEN_LPAREN, "(");
        break;
    case ')':
        addToken(ctx, TOKEN_RPAREN, ")");
        break;
    case '.':
        addToken(ctx, TOKEN_DOT, ".");
        break;

    // Operators
    case '!':
        if (matchChar(currentPosOfLexeme, '=')) {
            currentPosOfLexeme += 1;
            addToken(ctx, TOKEN_OPERATOR_NEQ, "!=");
            break;
        } else {
            fprintf(stderr, "\nUnrecognised Input");
            exit(1);
            break;
        }
    case '=':
        addToken(ctx, TOKEN_OPERATOR_EQ, "=");
        break;
    case '<':
        if (matchChar(currentPosOfLexeme, '=')) {
            currentPosOfLexeme += 1;
            addToken(ctx, TOKEN_OPERATOR_LTE, "<=");
        } else {
            addToken(ctx, TOKEN_OPERATOR_LT, "<");
        }
        break;
    case '>':
        if (matchChar(currentPosOfLexeme, '=')) {
            currentPosOfLexeme += 1;
            addToken(ctx, TOKEN_OPERATOR_GTE, ">=");
        } else {
            addToken(ctx, TOKEN_OPERATOR_GT, ">");
        }
        break;
    case '+':
        addToken(ctx, TOKEN_OPERATOR_PLUS, "+");
        break;
    case '-':
        addToken(ctx, TOKEN_OPERATOR_MINUS, "-");
        break;
    case '*':
        addToken(ctx, TOKEN_OPERATOR_STAR, "*");
        break;
    case '/':
        addToken(ctx, TOKEN_OPERATOR_SLASH, "/");
        break;

    // STRING LITERALS
    case '\'':
        currentPosOfLexeme = stringLiteral(currentPosOfLexeme);
        addToken(ctx, TOKEN_STRING_LITERAL,
                 getStringLiteral(currentPosOfLexeme, bufferStart));
        break;
    default:
        // INTEGER + FLOAT LITERALS
        if (isDigit(c)) {
            currentPosOfLexeme = numberLiteral(currentPosOfLexeme);
            char *numberLiteral =
                getNumberLiteral(currentPosOfLexeme, bufferStart);
            if (checkFloat(numberLiteral)) {
                addToken(ctx, TOKEN_FLOAT_LITERAL, numberLiteral);
            } else {
                addToken(ctx, TOKEN_INTEGER_LITERAL, numberLiteral);
            }
            currentPosOfLexeme -= 1;
            // assuming that any identifier will begin with a character
        } else if (isAlpha(c)) {
            currentPosOfLexeme = identifier(currentPosOfLexeme);

            char *lexeme =
                getIdentifierLiteral(currentPosOfLexeme, bufferStart);

            capitaliseString(lexeme);

            if (strcmp(lexeme, "EXIT") == 0) {
                free(lexeme);
                destroyTokenList(ctx);
                exit(0);
            }

            bool found = false;
            for (int i = 0; i < (sizeof(keywords) / sizeof(Keyword)); i++) {
                if (strcmp(keywords[i].keyword, lexeme) == 0) {
                    addToken(ctx, keywords[i].type, lexeme);
                    found = true;
                    break;
                }
            }

            if (!found) {
                free(lexeme);
                char *lexemeIdentifier =
                    getIdentifierLiteral(currentPosOfLexeme, bufferStart);
                addToken(ctx, TOKEN_IDENTIFIER, lexemeIdentifier);
            }
            currentPosOfLexeme -= 1;
        } else {
            fprintf(stderr, "\nUnrecognised Input");
            destroyTokenList(ctx);
            exit(1);
        }
        break;
    }

    return (currentPosOfLexeme + 1);
};

void addToken(tokenListCTX *ctx, TokenType tokenType, char *lexeme) {
    Token *token = malloc(sizeof(Token));
    token->type = tokenType;
    token->lexeme = lexeme;
    token->self = token;
    appendToken(token, ctx);
    return;
};

bool isAtEnd(char *posInBuffer) {
    if (*posInBuffer == '\n' || *posInBuffer == '\0' || *posInBuffer == '\r') {
        return true;
    }
    return false;
};

bool matchChar(char *posInBuffer, char expectedChar) {
    posInBuffer += 1;
    if (*posInBuffer == expectedChar) {
        return true;
    }
    return false;
};

char *stringLiteral(char *currentPosOfLexeme) {
    currentPosOfLexeme += 1;
    while (!isAtEnd(currentPosOfLexeme) && currentPosOfLexeme[0] != '\'') {
        currentPosOfLexeme += 1;
    }

    if (isAtEnd(currentPosOfLexeme)) {
        fprintf(stderr, "\nUnterminated string literal");
        exit(0);
    }
    return currentPosOfLexeme;
}

char *getStringLiteral(char *currentPosOfLexeme, char *startOfLexeme) {
    size_t len = (&currentPosOfLexeme[0] - startOfLexeme);
    currentPosOfLexeme -= len;
    char *string = malloc(sizeof(char) * len);
    size_t index = 0;
    currentPosOfLexeme += 1;
    while (currentPosOfLexeme[0] != '\'') {
        string[index] = currentPosOfLexeme[0];
        currentPosOfLexeme += 1;
        index += 1;
    }
    string[index] = '\0';
    return string;
};

bool isDigit(char c) {
    if (c >= '0' && c <= '9') {
        return true;
    }
    return false;
};

char *numberLiteral(char *currentPosOfLexeme) {
    while (!isAtEnd(currentPosOfLexeme) &&
           (isDigit(currentPosOfLexeme[0]) || currentPosOfLexeme[0] == '.')) {
        currentPosOfLexeme += 1;
    }

    if (currentPosOfLexeme[0] == '.') {
        fprintf(stderr, "\nUnrecognised number format.");
        exit(0);
    }

    return currentPosOfLexeme;
};

char *getNumberLiteral(char *currentPosOfLexeme, char *startOfLexeme) {
    size_t len = (&currentPosOfLexeme[0] - startOfLexeme) + 1;
    currentPosOfLexeme -= len - 1;
    char *number = malloc(sizeof(char) * len);
    size_t index = 0;
    size_t numOfDecimals = 0;
    while (isDigit(currentPosOfLexeme[0]) || currentPosOfLexeme[0] == '.') {
        if (currentPosOfLexeme[0] == '.') {
            numOfDecimals += 1;
        }
        number[index] = currentPosOfLexeme[0];
        currentPosOfLexeme += 1;
        index += 1;
    }
    number[index] = '\0';
    if (numOfDecimals > 1) {
        fprintf(stderr, "\nUnrecognised number format.");
        exit(0);
    }
    return number;
};

bool checkFloat(char *numberLiteral) {
    for (size_t i = 0; i < strlen(numberLiteral); i++) {
        if (numberLiteral[i] == '.') {
            return true;
        }
    }
    return false;
};

bool isAlpha(char c) {
    if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z')) {
        return true;
    }
    return false;
};

bool isAlphaNumeric(char c) { return isDigit(c) || isAlpha(c); };

char *identifier(char *currentPosOfLexeme) {
    while (isAlphaNumeric(currentPosOfLexeme[0])) {
        currentPosOfLexeme += 1;
    }
    return currentPosOfLexeme;
};

char *getIdentifierLiteral(char *currentPosOfLexeme, char *startOfLexeme) {
    size_t len = (&currentPosOfLexeme[0] - startOfLexeme) + 1;
    currentPosOfLexeme -= len - 1;
    char *identifier = malloc(sizeof(char) * len);
    size_t index = 0;
    while (isAlphaNumeric(currentPosOfLexeme[0])) {
        identifier[index] = currentPosOfLexeme[0];
        currentPosOfLexeme += 1;
        index += 1;
    }
    identifier[index] = '\0';
    return identifier;
};

void capitaliseString(char *str) {
    while (*str) {
        *str = toupper((unsigned char)*str);
        str++;
    }
}
