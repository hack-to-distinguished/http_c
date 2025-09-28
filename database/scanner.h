#include "token_list.h"
#include <stdbool.h>

void scanTokens(char *buffer);
char *scanToken(char *currentPosOfLexeme, tokenListCTX *ctx);
void addToken(tokenListCTX *ctx, TokenType tokenType, char *lexeme);
bool isAtEnd(char *posInBuffer);
bool matchChar(char *currentPosOfLexeme, char expectedChar);
char *stringLiteral(char *currentPosOfLexeme);
