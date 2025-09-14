#include "input_buffer.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {

    // tokenListCTX *ctx = initialiseTokenList(5);
    //
    // printf("\nTail Address: %p, Index Address: %p", ctx->tail,
    //        ctx->indexPosition);
    //
    // for (int i = 0; i < 8; i++) {
    //     Token token;
    //     token.line = i + 1;
    //     appendToken(token, ctx);
    // }
    //
    // for (int i = 0; i < 8; i++) {
    //     printf("\n%ld", (size_t)ctx->tail[i].line);
    // }

    while (true) {
        inputLineBuffer *iPL = createInputLineBuffer();
        getLineInput(iPL);

        processLineInput(iPL);
        destroyInputLineBuffer(iPL);
    }
    return 0;
}
