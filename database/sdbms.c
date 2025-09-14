#include "input_buffer.h"
#include "token_list.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {

    tokenListCTX *ctx = initialiseTokenList(5);

    Token token1;
    token1.line = 1;
    ctx->indexPosition = &token1;
    ctx->indexPosition += 1;

    Token token2;
    token2.line = 2;
    ctx->indexPosition = &token2;
    ctx->indexPosition += 1;

    Token token3;
    token3.line = 3;
    ctx->indexPosition = &token3;
    // ctx->indexPosition += 1;
    printf("\n%ld", ctx->indexPosition->line);

    // for (int i = 0; i < ctx->maxSize; i++) {
    //     Token token = ctx->tail[i];
    //     printf("\nToken Line: %ld, Token Address: %p", token.line, &token);
    // }
    // while (true) {
    //     inputLineBuffer *iPL = createInputLineBuffer();
    //     getLineInput(iPL);
    //
    //     processLineInput(iPL);
    //     destroyInputLineBuffer(iPL);
    // }
    return 0;
}
