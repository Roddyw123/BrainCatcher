#ifndef C_PARSER_H
#define C_PARSER_H

#include <stddef.h>

#include "tokenise.h"

typedef struct C_Atom {
    C_Type type;
    char *string;
    char val;
    struct C_Expr *offset;
} C_Atom;

typedef enum {
    C_EXPR_EQUALS, C_EXPR_PLUS, C_EXPR_MINUS,
    C_EXPR_SINGLE, C_EXPR_GREATER, C_EXPR_SMALLER,
    C_EXPR_NOT, C_EXPR_INC, C_EXPR_DEC, C_EXPR_MULT
} C_Expr_Type;

typedef struct C_Expr {
    C_Atom *atom1;
    C_Atom *atom2;
    C_Expr_Type type;
} C_Expr;

/////////////////////////////////////////////////////////////////////////

typedef enum {
    ATOM, EXPR, BLOCK
} C_operand_type;

typedef union {
    C_Atom *atom;
    C_Expr *expr;
    struct C_Block *block;
} C_Stmt_operand;

typedef enum {
    C_STMT_DECLARE, 
    C_STMT_SETEQUALS, 
    C_STMT_IF, 
    C_STMT_IFELSE, 
    C_STMT_WHILE,
    C_STMT_FOR,
    C_STMT_PUTCHAR,
    C_STMT_DECLAREARR,
    C_STMT_EXPR
} C_Stmt_Type;

typedef struct C_Stmt {
    C_Stmt_Type type;
    C_Stmt_operand operands[4]; // 4 because for loops have 4 components:
    C_operand_type optypes[4];  // init, cond, update, body
    size_t numOperands;
} C_Stmt;

/////////////////////////////////////////////////////////////////////////

typedef struct C_Block {
    C_Stmt *stmt;
    struct C_Block *next;
} C_Block;

/////////////////////////////////////////////////////////////////////////

C_Atom *parse_atom(C_Token *tokens, C_Token **nexttoken);
void C_print_atom(C_Atom *atom);
void C_free_atom(C_Atom *atom);

C_Expr *parse_expr(C_Token *tokens, C_Token **nexttoken);
void C_print_expr(C_Expr *expr);
void C_free_expr(C_Expr *expr);

C_Stmt *parse_stmt(C_Token *tokens, C_Token **nexttoken);
void C_print_stmt(C_Stmt *stmt);
void C_free_stmt(C_Stmt *stmt);

C_Block *parse_block(C_Token *tokens, C_Token **nexttoken);
void C_print_block(C_Block *block);
void C_free_block(C_Block *block);
#endif