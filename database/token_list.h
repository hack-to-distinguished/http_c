#include "tokens.h"
#include <stddef.h>

typedef struct {
    Token **index;
} tokenListCTX;

tokenListCTX *initialiseTokenList(size_t size);
void appendToken();
void getItem(size_t index);
void destroyTokenList();
