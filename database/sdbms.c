#include "input_buffer.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {

    while (true) {
        inputLineBuffer *iPL = createInputLineBuffer();
        getLineInput(iPL);

        processLineInput(iPL);
        destroyInputLineBuffer(iPL);
    }

    return 0;
}
