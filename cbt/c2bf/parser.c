#include "tokenise.h"
#include "parser.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#define _POSIX_C_SOURCE 200809L

#define fail_if_not_eq(x, y, z) if (x != y){\
    fprintf(stderr, "[parser]: %d is not equal to %d at %s\n", x, y, z);\
    exit(1);}

C_Atom *parse_atom_helper(
    C_Token *tokens,
    C_Token **nexttoken,
    bool isDeclaration
) {
    fail_if_null(tokens);
    C_Atom *atom = malloc(sizeof(C_Atom)); fail_if_null(atom);
    switch (tokens->type) {
        case C_VALUE:
            atom->type = C_VALUE;
            atom->val  = tokens->value;
            atom->offset = NULL;
            *nexttoken = tokens->next;

            break;
        case C_IDENT:
            atom->type = C_IDENT;
            atom->string = strdup(tokens->string); fail_if_null(atom->string);
            tokens = tokens->next;
            if (tokens->type != C_LSQUARE || isDeclaration) {
                *nexttoken = tokens;
                break;
            } else {
        // c [ <expr> ]
                C_Expr *offset = parse_expr(tokens->next, &tokens);
                atom->offset = offset;
                fail_if_not_eq(tokens->type, C_RSQUARE, "missing ] in parse_atom");
                tokens = tokens->next;
                *nexttoken = tokens;
            }
            break;
        case C_GETCHAR:
            atom->type = C_GETCHAR;
            tokens = tokens->next;
            fail_if_not_eq(tokens->type, C_LBRAC, "[parse atom]: expect ( after getchar");
            tokens = tokens->next;
            fail_if_not_eq(tokens->type, C_RBRAC, "[parse atom]: expect ) after getchar(");
            *nexttoken = tokens->next;
            break;
        default:
            fail("Cannot identify C_Atom in parse_atom");
    }
    return atom;
}

C_Atom *parse_atom(C_Token *tokens, C_Token **nexttoken) {
    return parse_atom_helper(tokens, nexttoken, false);
}

C_Atom *parse_atom_decl(C_Token *tokens, C_Token **nexttoken) {
    return parse_atom_helper(tokens, nexttoken, true);
}

void C_print_atom(C_Atom *atom) {
    fail_if_null(atom);
    switch(atom->type) {
        case C_VALUE:
            printf("[VAL:  %d]", atom->val);
            break;
        case C_IDENT:
            if (atom->offset == NULL) {
                printf("[IDEN: %s]", atom->string);
            } else {
                printf("[ARR: %s ", atom->string);
                C_print_expr(atom->offset);
                printf("]");
            }
            break;
        case C_GETCHAR:
            printf("[GETCHAR]");
            break;
        default:
            fail("Cannot identify C_Atom in C_print_atom");
    }
}

void C_free_atom(C_Atom *atom) {
    fail_if_null(atom);
    switch(atom->type) {
        case C_VALUE:
            break;
        case C_IDENT:
            free(atom->string);
            break;
        case C_GETCHAR:
            break;
        default:
            fail("Cannot identify C_Atom in C_free_atom");
    }
    free(atom);
}

C_Expr *parse_expr(C_Token *tokens, C_Token **nexttoken) {
    C_Expr *expr  = calloc(1, sizeof *expr); fail_if_null(expr);
    C_Atom *atom1, *atom2;
    if (tokens->type == C_NOT) {
        expr->atom1 = parse_atom(tokens->next, &tokens);
        expr->type  = C_EXPR_NOT;
        *nexttoken  = tokens;
        return expr;
    }
    atom1 = parse_atom(tokens, &tokens);
    switch(tokens->type) {
        case C_EQUALS:
            atom2 = parse_atom(tokens->next, &tokens);
            expr->atom1 = atom1;
            expr->atom2 = atom2;
            expr->type  = C_EXPR_EQUALS;
            break;
        case C_GREATER:
            atom2 = parse_atom(tokens->next, &tokens);
            expr->atom1 = atom1;
            expr->atom2 = atom2;
            expr->type  = C_EXPR_GREATER;
            break;
        case C_SMALLER:
            atom2 = parse_atom(tokens->next, &tokens);
            expr->atom1 = atom1;
            expr->atom2 = atom2;
            expr->type  = C_EXPR_SMALLER;
            break;
        case C_PLUS:
            atom2 = parse_atom(tokens->next, &tokens);
            expr->atom1 = atom1;
            expr->atom2 = atom2;
            expr->type  = C_EXPR_PLUS;
            break;
        case C_MINUS:
            atom2 = parse_atom(tokens->next, &tokens);
            expr->atom1 = atom1;
            expr->atom2 = atom2;
            expr->type = C_EXPR_MINUS;
            break;
        case C_INC:
            expr->atom1 = atom1;
            expr->type = C_EXPR_INC;
            tokens = tokens->next;
            break;
        case C_DEC:
            expr->atom1 = atom1;
            expr->type = C_EXPR_DEC;
            tokens = tokens->next;
            break;
        case C_MULT:
            atom2 = parse_atom(tokens->next, &tokens);
            expr->atom1 = atom1;
            expr->atom2 = atom2;
            expr->type = C_EXPR_MULT;
            break;
        default:
            expr->atom1 = atom1;
            expr->type = C_EXPR_SINGLE;
            break;
    }
    *nexttoken = tokens;
    return expr;
}
void C_print_expr(C_Expr *expr) {
    printf("Expr: ");
    switch(expr->type) {
        case C_EXPR_SINGLE:
            C_print_atom(expr->atom1);
            break;
        case C_EXPR_EQUALS:
            C_print_atom(expr->atom1);
            printf(" == ");
            C_print_atom(expr->atom2);
            break;
        case C_EXPR_PLUS:
            C_print_atom(expr->atom1);
            printf(" + ");
            C_print_atom(expr->atom2);
            break;
        case C_EXPR_GREATER:
            C_print_atom(expr->atom1);
            printf(" > ");
            C_print_atom(expr->atom2);
            break;
        case C_EXPR_SMALLER:
            C_print_atom(expr->atom1);
            printf(" < ");
            C_print_atom(expr->atom2);
            break;
        case C_EXPR_NOT:
            printf(" !");
            C_print_atom(expr->atom1);
            break;
        case C_EXPR_MINUS:
            C_print_atom(expr->atom1);
            printf(" - ");
            C_print_atom(expr->atom2);
            break;
        case C_EXPR_INC:
            C_print_atom(expr->atom1);
            printf("++");
            break;
        case C_EXPR_DEC:
            C_print_atom(expr->atom1);
            printf("--");
            break;
        case C_EXPR_MULT:
            C_print_atom(expr->atom1);
            printf(" * ");
            C_print_atom(expr->atom2);
            break;
        default:
            fail("Cannot identify C_Expr");
    }
}
void C_free_expr(C_Expr *expr) {
    fail_if_null(expr);
    C_free_atom(expr->atom1);
    if (expr->type != C_EXPR_SINGLE && expr->type != C_EXPR_NOT &&
        expr->type != C_EXPR_INC    && expr->type != C_EXPR_DEC) {
        C_free_atom(expr->atom2);
    }
    free(expr);
}

C_Stmt *parse_stmt(C_Token *tokens, C_Token **nexttoken) {
    if (tokens == NULL) return NULL;
    C_Stmt *stmt = calloc(1, sizeof *stmt); fail_if_null(stmt);
    // stmt must have format:
    // 1. char IDEN;
    // 2. char IDEN = EXPR; (where expr could be getchar)
    // 3. IDEN = EXPR;
    // 4. if (EXPR) {BLOCK}
    // 5. while (EXPR) {BLOCK}
    
    //           BLOCK            EXPR   BLOCK/EXPR   BLOCK
    // 6. for (char IDEN = EXPR; EXPR; IDEN = EXPR) {BLOCK}
    // 6a. 1st EXPR = PLUS, MINUS, SINGLE, NOT, MULT
    // 6b. 2nd EXPR = EQUALS, SINGLE, NOT, SMALLER, GREATER
    // 6c. 3rd EXPR = INC, DEC (no IDEN), SINGLE, MINUS, PLUS, MULT
    // 7. putchar(ATOM);  (cus atom could be ident, or nat)
    C_Atom *iden;
    C_Token *symbol;
    switch (tokens->type) {
        case C_FOR:
            // for (INIT; COND; UPDATE) { body }
            // 
            // 
            // INIT
            // while (cond)  {
            //      BODY;
            //      UPDATE;
            // }
            //
            // ASSUMPTIONS:
            // INIT only initializes 0 or 1 variables
            // COND is 0 or 1 expr that can be evaluated to true or false 
            // UPDATE only updates 0 or 1 variables

            tokens = tokens->next;
            fail_if_not_eq(tokens->type, C_LBRAC, "parse_stmt C_FOR C_LBRAC");

            // Check for init
            tokens = tokens->next;
            if(tokens->type != C_SEMICOLON) {
                C_Block *initBlock = malloc(sizeof(C_Block)); 
                fail_if_null(initBlock);

                C_Stmt *initDecl = calloc(1, sizeof *initDecl); 
                fail_if_null(initDecl);
                
                // first declare variable name (C_CHAR)
                C_Token *iden_token = tokens->next;
                iden = parse_atom(iden_token, &tokens);
                symbol = tokens;
                fail_if_null(symbol);
                initDecl->type = C_STMT_DECLARE;
                initDecl->operands[0].atom = iden;
                initDecl->optypes[0] = ATOM;
                initDecl->numOperands = 1;

                tokens = iden_token; // we want to read IDEN again;

                initBlock->stmt = initDecl;
                initBlock->next = NULL;
                
                // Then set that variable to init value (C_IDENT)
                C_Block *initBlock2 = malloc(sizeof(C_Block)); 
                fail_if_null(initBlock2);
                C_Stmt *initSetEq = calloc(1, sizeof *initSetEq); 
                fail_if_null(initSetEq);
                iden = parse_atom(tokens, &tokens);
                symbol = tokens;
                fail_if_not_eq(symbol->type, C_SETEQUALS, "parse_stmt C_FOR C_SETEQUALS");
                C_Expr *expr = parse_expr(tokens->next, &tokens);

                initSetEq->type = C_STMT_SETEQUALS;
                initSetEq->operands[0].atom = iden;
                initSetEq->optypes[0] = ATOM;
                initSetEq->operands[1].expr   = expr;
                initSetEq->optypes[1] = EXPR;
                initSetEq->numOperands = 2;
                initBlock2->stmt = initSetEq;
                initBlock->next = initBlock2;
                initBlock2->next = NULL;

                stmt->operands[0].block = initBlock;
                stmt->optypes[0] = BLOCK;
            } else {
                stmt->operands[0].block = NULL;
                stmt->optypes[0] = BLOCK;
            }

            // Check for cond (expr)
            tokens = tokens->next;
            if (tokens->type != C_SEMICOLON) { 
                C_Expr *expr = parse_expr(tokens, &tokens);
                fail_if_not_eq(tokens->type, C_SEMICOLON, "parse_stmt C_FOR C_SEMICOLON");

                stmt->operands[1].expr = expr;
                stmt->optypes[1] = EXPR;
            } else {
                stmt->operands[1].expr = NULL;
                stmt->optypes[1] = EXPR;
            }

            // Check for update
            tokens = tokens->next;
            if (tokens->type != C_RBRAC) {
                // block cus stmt operands can only take blocks 
                C_Block *updateBlock = malloc(sizeof(C_Block)); 
                fail_if_null(updateBlock);

                C_Stmt *update = calloc(1, sizeof *update); fail_if_null(update);
                
                // Set variable's value (C_IDENT)
                iden = parse_atom(tokens, &tokens);
                symbol = tokens;
                fail_if_not_eq(symbol->type, C_SETEQUALS, "parse_stmt C_FOR C_SETEQUALS");
                C_Expr *expr = parse_expr(tokens->next, &tokens);

                update->type = C_STMT_SETEQUALS;
                update->operands[0].atom = iden;
                update->optypes[0] = ATOM;
                update->operands[1].expr   = expr;
                update->optypes[1] = EXPR;
                update->numOperands = 2;

                updateBlock->stmt = update;
                updateBlock->next = NULL;

                stmt->operands[2].block = updateBlock;
                stmt->optypes[2] = BLOCK;
            } else {
                stmt->operands[2].block = NULL;
                stmt->optypes[2] = BLOCK;
            }
            tokens = tokens->next;

            // Body of the loop
            fail_if_not_eq(tokens->type, C_LCURL, "parse_stmt C_WHILE");
            tokens = tokens->next;
            C_Block *blockBody = parse_block(tokens, &tokens);
            fail_if_not_eq(tokens->type, C_RCURL, "parse_stmt C_WHILE");
            tokens = tokens->next;

            stmt->type = C_STMT_FOR;
            stmt->operands[3].block = blockBody;
            stmt->optypes[3] = BLOCK;
            stmt->numOperands = 4;
            *nexttoken = tokens;
            break;

        case C_CHAR:
            C_Token *iden_token = tokens->next;
            iden = parse_atom_decl(iden_token, &tokens);
            symbol = tokens;
            fail_if_null(symbol);
            stmt->type = C_STMT_DECLARE;
            stmt->operands[0].atom = iden;
            stmt->optypes[0] = ATOM;
            stmt->numOperands = 1;
            if (symbol->type == C_SEMICOLON) {
                *nexttoken = tokens->next;
            } else if (symbol->type == C_SETEQUALS){
                *nexttoken = iden_token; // we want to read IDEN again;
            } else if (symbol->type == C_LSQUARE){
                fail_if_not_eq(symbol->next->type, C_VALUE, 
                    "parse_stmt C_CHAR array");
                fail_if_not_eq(symbol->next->next->type, C_RSQUARE, 
                    "parse_stmt C_CHAR array");
                fail_if_not_eq(symbol->next->next->next->type, C_SEMICOLON, 
                    "parse_stmt C_CHAR array");

                stmt->type = C_STMT_DECLAREARR;
                stmt->operands[1].atom = parse_atom(symbol->next, &tokens);
                stmt->optypes[1] = ATOM;
                stmt->numOperands = 2;
                *nexttoken = tokens->next->next;
            } else {
                fail("Stmt must end with semicolon");
            }
            break;
        case C_IDENT:
            C_Token *oldToken = tokens;
            C_Expr *expr;
            iden = parse_atom(tokens, &tokens);
            symbol = tokens;
            if (symbol->type == C_SETEQUALS) {
                expr = parse_expr(tokens->next, &tokens);
                fail_if_not_eq(tokens->type, C_SEMICOLON, "parse_stmt C_IDENT");
                stmt->type = C_STMT_SETEQUALS;
                stmt->operands[0].atom = iden;
                stmt->optypes[0] = ATOM;
                stmt->operands[1].expr   = expr;
                stmt->optypes[1] = EXPR;
                stmt->numOperands = 2;
            } else if (symbol->type == C_INC || symbol->type == C_DEC) {
                expr = parse_expr(oldToken, &tokens);
                fail_if_not_eq(tokens->type, C_SEMICOLON, "parse_stmt C_IDENT");
                stmt->type = C_STMT_EXPR;
                stmt->operands[0].expr = expr;
                stmt->optypes[0] = EXPR; 
                stmt->numOperands = 1;
            } else {
                fail("token not recognized in parse_stmt");
            }
            *nexttoken = tokens->next;
            break;
        // C_IF is of format:
        // if (<expr>) {<block>}
        case C_IF:
            tokens = tokens->next;
            fail_if_not_eq(tokens->type, C_LBRAC, "parse_stmt C_IF");
            tokens = tokens->next;
            C_Expr *cond = parse_expr(tokens, &tokens);
            fail_if_not_eq(tokens->type, C_RBRAC, "parse_stmt C_IF");
            tokens = tokens->next;
            fail_if_not_eq(tokens->type, C_LCURL, "parse_stmt C_IF");
            tokens = tokens->next;
            C_Block *true_block = parse_block(tokens, &tokens);
            fail_if_not_eq(tokens->type, C_RCURL, "parse_stmt C_IF");
            tokens = tokens->next;
            stmt->operands[0].expr = cond;
            stmt->optypes[0] = EXPR;
            stmt->operands[1].block = true_block;
            stmt->optypes[1] = BLOCK;
            if (tokens == NULL || tokens->type != C_ELSE) {
                stmt->type = C_STMT_IF;
                stmt->numOperands = 2;
            } else {
                tokens = tokens->next;
                fail_if_not_eq(tokens->type, C_LCURL, "parse_stmt C_IF");
                tokens = tokens->next;
                C_Block *false_block = parse_block(tokens, &tokens);
                fail_if_not_eq(tokens->type, C_RCURL, "parse_stmt C_IF");
                stmt->type = C_STMT_IFELSE;
                stmt->operands[2].block = false_block;
                stmt->optypes[2] = BLOCK;
                stmt->numOperands = 3;
            }
            *nexttoken = tokens;
            break;
        // C_WHILE format:
        // while (<expr>) {<block>}
        case C_WHILE:
            tokens = tokens->next;
            fail_if_not_eq(tokens->type, C_LBRAC, "parse_stmt C_WHILE");
            tokens = tokens->next;
            C_Expr *cond1 = parse_expr(tokens, &tokens);
            fail_if_not_eq(tokens->type, C_RBRAC, "parse_stmt C_WHILE");
            tokens = tokens->next;
            fail_if_not_eq(tokens->type, C_LCURL, "parse_stmt C_WHILE");
            tokens = tokens->next;
            C_Block *block = parse_block(tokens, &tokens);
            fail_if_not_eq(tokens->type, C_RCURL, "parse_stmt C_WHILE");
            tokens = tokens->next;

            stmt->type = C_STMT_WHILE;
            stmt->operands[0].expr = cond1;
            stmt->optypes[0] = EXPR;
            stmt->operands[1].block = block;
            stmt->optypes[1] = BLOCK;
            stmt->numOperands = 2;
            *nexttoken = tokens;
            break;
        
        // putchar(EXPR) where expr can be evaluated to a nat 0..255
        case C_PUTCHAR:
            tokens = tokens->next;
            fail_if_not_eq(tokens->type, C_LBRAC, 
                "parse_stmt: expect ( after putchar");
            expr = parse_expr(tokens->next, &tokens);
            fail_if_not_eq(tokens->type, C_RBRAC, 
                "parse_stmt: expect ) after expr C_PUTCHAR");
            tokens = tokens->next;

            stmt->type = C_STMT_PUTCHAR;
            stmt->operands[0].expr = expr;
            stmt->optypes[0] = EXPR;
            stmt->numOperands = 1;
            *nexttoken = tokens->next;
            break;

        default:
            fprintf(stderr, "Token: %d\n", tokens->type);
            fail("Cannot identify C_Stmt in parse_stmt");
    }

    return stmt;
}

void C_print_stmt(C_Stmt *stmt) {
    fail_if_null(stmt);
    printf("Stmt: ");
    switch (stmt->type) {
        case C_STMT_DECLARE:
            printf("char %s;\n", stmt->operands[0].atom->string);
            break;
        case C_STMT_DECLAREARR:
            printf("char %s[%d];\n", 
                stmt->operands[0].atom->string, stmt->operands[1].atom->val);
            break;
        case C_STMT_SETEQUALS:
            C_print_atom(stmt->operands[0].atom);
            printf("= ");
            C_print_expr(stmt->operands[1].expr);
            printf(";\n");
            break;
        case C_STMT_IF:
            printf("if (");
            C_print_expr(stmt->operands[0].expr);
            printf(") {\n");
            C_print_block(stmt->operands[1].block);
            printf("}\n");
            break;
        case C_STMT_IFELSE:
            printf("if (");
            C_print_expr(stmt->operands[0].expr);
            printf(") {\n");
            C_print_block(stmt->operands[1].block);
            printf("} else {\n");
            C_print_block(stmt->operands[1].block);
            printf("}\n");
            break;
        case C_STMT_WHILE:
            printf("while (");
            C_print_expr(stmt->operands[0].expr);
            printf(") {\n");
            C_print_block(stmt->operands[1].block);
            printf("}\n");
            break;
        case C_STMT_FOR:
            printf("for (\n");
            if (stmt->operands[0].block != NULL) C_print_block(stmt->operands[0].block);
            if (stmt->operands[1].expr != NULL) C_print_expr(stmt->operands[1].expr);
            if (stmt->operands[2].block != NULL) C_print_block(stmt->operands[2].block);
            printf(") {\n");
            C_print_block(stmt->operands[3].block);
            printf("}\n");
            break;
        case C_STMT_PUTCHAR:
            printf("Putchar(");
            C_print_expr(stmt->operands[0].expr);
            printf(");\n");
            break;
        default:
            fail("Cannot identify C_Stmt");
    }
}

void C_free_stmt(C_Stmt *stmt) {
    fail_if_null(stmt);
    for (int i = 0; i < stmt->numOperands; i++) {
        
        switch(stmt->optypes[i]) {
            case ATOM:
                // if (stmt->operands[i].atom == NULL) continue; 
                C_free_atom(stmt->operands[i].atom);
                break;
            case EXPR:
                if (stmt->operands[i].expr == NULL) continue; // for loop expr might be null
                C_free_expr(stmt->operands[i].expr);
                break;
            case BLOCK:
                if (stmt->operands[i].block == NULL) continue; // for loop blocks might be null
                C_free_block(stmt->operands[i].block);
                break;
        }
    }
    free(stmt);
}

C_Block *parse_block(C_Token *tokens, C_Token **nexttoken) {
    fail_if_null(tokens);
    C_Block *head = malloc(sizeof(C_Block)); fail_if_null(head);
    C_Block *curr = head;
    C_Block *next;

    C_Token *currtoken = tokens;
    while (currtoken != NULL && currtoken->type != C_RCURL) {
        next = malloc(sizeof(C_Block)); fail_if_null(next);
        next->stmt = parse_stmt(currtoken, &currtoken);
        curr->next = next;
        curr = curr->next;
    }
    curr->next = NULL;
    *nexttoken = currtoken; 

    C_Block *result = head->next;
    free(head);
    return result;
}

void C_print_block(C_Block *block) {
    fail_if_null(block);
    C_Block *curr = block;
    printf("Block:\n");
    while (curr != NULL) {
        C_print_stmt(curr->stmt);
        curr = curr->next;
    }
}

void C_free_block(C_Block *block) {
    fail_if_null(block);
    C_Block *curr = block;
    C_Block *next;
    while (curr != NULL) {
        next = curr->next;
        C_free_stmt(curr->stmt);
        free(curr);
        curr = next;
    }
}