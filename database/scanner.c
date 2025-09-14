#include "scanner.h"
#include "token_list.h"
#include <stdlib.h>

void scanTokens(char *buffer) {
    tokenListCTX *ctx = malloc(sizeof(tokenListCTX));
    initialiseTokenList(8);
    int lineNumber = 1;
    return;
}
