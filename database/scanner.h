#include "token_list.h"
#include <stdbool.h>

void scanTokens(char *buffer);
char *scanToken(char *currentPosOfLexeme, tokenListCTX *ctx);
void addToken(tokenListCTX *ctx, TokenType tokenType);
bool isAtEnd(char *posInBuffer);
bool match(char *currentPostOfLexeme, char expectedChar);
