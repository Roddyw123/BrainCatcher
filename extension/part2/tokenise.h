#ifndef C_TOKENISE_H
#define C_TOKENISE_H

#include <stdbool.h>
#include <string.h>
#define STREQ(x, y) !strcmp(x, y)
#define fail_if_null(ptr) if (ptr == NULL){\
    fprintf(stderr, "Failed to malloc\n");\
    exit(1);}
#define fail(msg) fprintf(stderr, "%s\n", msg); exit(1)

typedef enum {
    C_SETEQUALS,
    C_SEMICOLON,
    C_VALUE,
    C_IDENT,
    C_CHAR,
    C_IF,
    C_ELSE,
    C_WHILE,
    C_FOR,
    C_LBRAC,
    C_RBRAC,
    C_LCURL,
    C_RCURL,
    C_LSQUARE,
    C_RSQUARE,
    C_EQUALS,
    C_PLUS,
    C_MINUS,
    C_INC,
    C_DEC,
    C_MULT,
    C_NOT,
    C_GREATER,
    C_SMALLER,
    C_GETCHAR,
    C_PUTCHAR,
} C_Type;

typedef struct C_Token {
    char *string;
    char value;
    C_Type type;
    struct C_Token *next;
} C_Token;

extern const char *C_Keywords[];

extern const int num_c_keywords;

bool isKeyword(char *word);
C_Token *c_tokenise(char *buffer);
void C_print_tokens(C_Token *token);
void C_free_tokens(C_Token *token);
#endif