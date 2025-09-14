#include "tokens.h"
#include <stddef.h>

typedef struct {
    Token *indexPosition;
    Token *tail;
    size_t maxSize;
} tokenListCTX;

tokenListCTX *initialiseTokenList(size_t size);
void appendToken(Token token);
void getItem(size_t index);
void destroyTokenList();
