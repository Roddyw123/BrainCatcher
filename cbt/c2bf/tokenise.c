#include "tokenise.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BASE 10
#define MAX_KEYWORD_NAME_LEN 31
#define PUTCHARLEN 6
#define GETCHARLEN 6

const char *C_Keywords[] = {
    "auto",       "break",     "case",           "char",
    "const",      "continue",  "default",        "do",
    "double",     "else",      "enum",           "extern",
    "float",      "for",       "goto",           "if",
    "inline",     "int",       "long",           "register",
    "restrict",   "return",    "short",          "signed",
    "sizeof",     "static",    "struct",         "switch",
    "typedef",    "union",     "unsigned",       "void",
    "volatile",   "while",     "_Alignas",       "_Alignof",
    "_Atomic",    "_Bool",     "_Complex",       "_Generic",
    "_Imaginary", "_Noreturn", "_Static_assert", "_Thread_local"};

const int num_c_keywords = sizeof(C_Keywords) / sizeof(C_Keywords[0]);

// PRE: word is NUL terminated
bool isKeyword(char *word) {
    for (int i = 0; i < num_c_keywords; i++) {
        if (STREQ(word, C_Keywords[i])) return true;
    }
    return false;
}
//
// PRE: buffer is NUL terminated
C_Token *c_tokenise(char *buffer) {
    C_Token *head = malloc(sizeof(C_Token));
    fail_if_null(head);
    C_Token *curr = head;
    C_Token *next;
    for (char *chrptr = buffer; *chrptr != '\0'; chrptr++) {
        // Skip whitespace
        if (isspace(*chrptr)) continue;
        next = malloc(sizeof(C_Token));
        fail_if_null(next);
        switch (*chrptr) {
            case '=':
                if (*(chrptr + 1) == '=') {
                    next->type = C_EQUALS;
                    chrptr++;
                } else {
                    next->type = C_SETEQUALS;
                }
                break;
            case ';':
                next->type = C_SEMICOLON;
                break;
            case '(':
                next->type = C_LBRAC;
                break;
            case ')':
                next->type = C_RBRAC;
                break;
            case '{':
                next->type = C_LCURL;
                break;
            case '}':
                next->type = C_RCURL;
                break;
            case '[':
                next->type = C_LSQUARE;
                break;
            case ']':
                next->type = C_RSQUARE;
                break;
            case '+':
                if (*(chrptr + 1) != '+') {
                    next->type = C_PLUS;
                } else {
                    next->type = C_INC;
                    chrptr++;
                }
                break;
            case '-':
                if (*(chrptr + 1) != '-') {
                    next->type = C_MINUS;
                } else {
                    next->type = C_DEC;
                    chrptr++;
                }
                break;
            case '*':
                next->type = C_MULT;
                break;
            case '!':
                next->type = C_NOT;
                break;
            case '>':
                next->type = C_GREATER;
                break;
            case '<':
                next->type = C_SMALLER;
                break;
            default:
                if (isdigit(*chrptr)) {  // NATURAL NUMBERS
                    int num = strtol(chrptr, &chrptr, BASE);
                    if (num < 0 || num > 255) {
                        fail("Only char number supported");
                    }
                    next->type = C_VALUE;
                    next->value = num;
                    chrptr--;  // chrptr does not need to be incremented
                    break;
                } else if (strncmp(chrptr, "getchar", 7) == 0) {  // Get char
                    next->type = C_GETCHAR;
                    chrptr += GETCHARLEN;
                    break;
                } else if (strncmp(chrptr, "putchar", 7) == 0) {
                    next->type = C_PUTCHAR;
                    chrptr += PUTCHARLEN;
                    break;
                } else {  // KEYWORDS
                    int len = 0;
                    while (isalnum(*(chrptr + len))) len++;
                    if (len == 0) {  // Unrecognized symbol, skip
                        free(next);
                        continue;
                    }
                    if (len >= MAX_KEYWORD_NAME_LEN) {
                        fprintf(stderr,
                                "Keyword is too long (exceeds 99 chars)\n");
                        exit(EXIT_FAILURE);
                    }
                    char temp[MAX_KEYWORD_NAME_LEN];
                    strncpy(temp, chrptr, len);
                    temp[len] = '\0';
                    if (isKeyword(temp)) {
                        if (STREQ("char", temp)) {
                            next->type = C_CHAR;
                        } else if (STREQ("if", temp)) {
                            next->type = C_IF;
                        } else if (STREQ("else", temp)) {
                            next->type = C_ELSE;
                        } else if (STREQ("while", temp)) {
                            next->type = C_WHILE;
                        } else if (STREQ("for", temp)) {
                            next->type = C_FOR;
                        }
                        else {
                            // fprintf(stderr, "Keyword not supported");
                            // exit(EXIT_FAILURE);
                        }
                    } else {
                        next->type = C_IDENT;
                        next->string =
                            malloc(sizeof(char) * MAX_KEYWORD_NAME_LEN);
                        fail_if_null(next->string);
                        strncpy(next->string, temp, MAX_KEYWORD_NAME_LEN);
                        // printf("Variable: %s\n", next->string);
                    }
                    chrptr +=
                        len - 1;  // minus one because the for loop also adds 1
                }
        }
        curr->next = next;
        curr = curr->next;
    }
    curr->next = NULL;
    C_Token *result = head->next;
    free(head);
    return result;
}

// PRE: tokens is NULL terminated
void C_print_tokens(C_Token *token) {
    fail_if_null(token);
    printf("Tokens:\n");
    while (token != NULL) {
        switch (token->type) {
            case C_CHAR:
                printf("TYPE: char\n");
                break;
            case C_GETCHAR:
                printf("TYPE: GETCHAR \n");
                break;
            case C_PUTCHAR:
                printf("TYPE: PUTCHAR \n");
                break;
            case C_IF:
                printf("CTRL: if\n");
                break;
            case C_ELSE:
                printf("CTRL: else\n");
                break;
            case C_WHILE:
                printf("CTRL: while\n");
                break;
            case C_FOR:
                printf("CTRL: for\n");
                break;
            case C_IDENT:
                printf("IDEN: %s\n", token->string);
                break;
            case C_SETEQUALS:
                printf("SYMB: =\n");
                break;
            case C_VALUE:
                printf("VAL:  %d\n", token->value);
                break;
            case C_SEMICOLON:
                printf("SYMB: ;\n");
                break;
            case C_LBRAC:
                printf("SYMB: (\n");
                break;
            case C_RBRAC:
                printf("SYMB: )\n");
                break;
            case C_LCURL:
                printf("SYMB: {\n");
                break;
            case C_RCURL:
                printf("SYMB: }\n");
                break;
            case C_LSQUARE:
                printf("SYMB: [\n");
                break;
            case C_RSQUARE:
                printf("SYMB: ]\n");
                break;
            case C_EQUALS:
                printf("SYMB: ==\n");
                break;
            case C_PLUS:
                printf("SYMB: +\n");
                break;
            case C_INC:
                printf("SYMB: ++\n");
                break;
            case C_NOT:
                printf("SYMB: !\n");
                break;
            case C_GREATER:
                printf("SYMB: >\n");
                break;
            case C_SMALLER:
                printf("SYMB: <\n");
                break;
            case C_MINUS:
                printf("SYMB: -\n");
                break;
            case C_DEC:
                printf("SYMB: --\n");
                break;
            case C_MULT:
                printf("SYMB: *\n");
                break;
            
        }
        token = token->next;
    }
}

void C_free_tokens(C_Token *token) {
    fail_if_null(token);
    C_Token *curr = token;
    C_Token *next;
    while (curr != NULL) {
        next = curr->next;
        if (curr->type == C_IDENT) free(curr->string);
        free(curr);
        curr = next;
    }
}

// int main(void) {
//     char *buffer = "char x = 1;";
//     C_Token *tokens = c_tokenise(buffer);
//     C_print_tokens(tokens);
//     C_free_tokens(tokens);
//     return EXIT_SUCCESS;
// }