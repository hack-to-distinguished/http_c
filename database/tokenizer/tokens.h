#include <stddef.h>
typedef enum {
    // Keywords
    // Just going to do simple SQL commands, no inner joins etc.
    TOKEN_KEYWORD_SELECT,
    TOKEN_KEYWORD_FROM,
    TOKEN_KEYWORD_WHERE,
    TOKEN_KEYWORD_INSERT,
    TOKEN_KEYWORD_UPDATE,
    TOKEN_KEYWORD_DELETE,
    TOKEN_KEYWORD_EXIT,
    TOKEN_KEYWORD_NULL,
    TOKEN_KEYWORD_AND,
    TOKEN_KEYWORD_OR,
    TOKEN_KEYWORD_ORDER,
    TOKEN_KEYWORD_BY,

    // Identifiers -> names of columns, tables, etc
    TOKEN_IDENTIFIER,

    // Literals
    TOKEN_STRING_LITERAL,
    TOKEN_INTEGER_LITERAL,
    TOKEN_FLOAT_LITERAL,

    // Operators
    TOKEN_OPERATOR_EQ,
    TOKEN_OPERATOR_NEQ,
    TOKEN_OPERATOR_LT,
    TOKEN_OPERATOR_LTE,
    TOKEN_OPERATOR_GT,
    TOKEN_OPERATOR_GTE,
    TOKEN_OPERATOR_PLUS,
    TOKEN_OPERATOR_MINUS,
    TOKEN_OPERATOR_STAR,
    TOKEN_OPERATOR_SLASH,

    // Punctuation
    TOKEN_COMMA,
    TOKEN_SEMICOLON,
    TOKEN_LPAREN,
    TOKEN_RPAREN,
    TOKEN_DOT,
} TokenType;

typedef struct Token {
    TokenType type;
    char *lexeme;
    size_t line;
    struct Token *self;
} Token;

typedef struct {
    char *keyword;
    TokenType type;
} Keyword;

static Keyword keywords[] = {
    {"SELECT", TOKEN_KEYWORD_SELECT}, {"INSERT", TOKEN_KEYWORD_INSERT},
    {"DELETE", TOKEN_KEYWORD_DELETE}, {"FROM", TOKEN_KEYWORD_FROM},
    {"UPDATE", TOKEN_KEYWORD_UPDATE}, {"WHERE", TOKEN_KEYWORD_WHERE},
    {"EXIT", TOKEN_KEYWORD_EXIT},     {"NULL", TOKEN_KEYWORD_NULL},
    {"AND", TOKEN_KEYWORD_AND},       {"OR", TOKEN_KEYWORD_OR},
    {"ORDER", TOKEN_KEYWORD_ORDER},   {"BY", TOKEN_KEYWORD_BY},

};
