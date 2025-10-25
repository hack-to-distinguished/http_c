#include "tokens.h"
#include <stdbool.h>
#include <stddef.h>

typedef struct {
    Token *indexPosition;
    Token *tail;
    size_t maxSize;
    size_t currentSize;
} tokenListCTX;

tokenListCTX *initialiseTokenList(size_t size);
bool isFull(tokenListCTX *ctx);
void appendToken(Token *token, tokenListCTX *ctx);
void getItem(size_t index);
void destroyTokenList(tokenListCTX *ctx);
void printAllTokens(tokenListCTX *ctx);
