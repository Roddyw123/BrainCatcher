#include "parser.h"
#include "variable-table.h"
#include "emitter.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define RESULT_INDEX 3
#define GOTO_INDEX 9
#define GOTO_TEMP 8
#define TEMP_INDEX 6

#define OFFSET_INDEX 6

// GREATER/LESS THAN
#define TEMP0 1
#define TEMP1 2
#define X 0
#define Y 4
#define Z 3

// PLUS/MINUS
#define EXPR_TEMP_INDEX 5

// NOT
#define NOT_X 7
#define NOT_Y 8



void goto_index(FILE *out, int *dataptr, int index) {
    // move right until *dataptr == index
    while (*dataptr < index) {
        fputc('>', out);
        (*dataptr)++;
    }
    // move left until *dataptr == index
    while (*dataptr > index) {
        fputc('<', out);
        (*dataptr)--;
    }
}



// goes to to index given by the M[cell]
void goto_index_cell(FILE *out, int *dataptr, int cell) {
    goto_index(out, dataptr, cell);
    set_equals_var(out, GOTO_TEMP, cell, dataptr);
    fprintf(out, "[--[>>+<<-]]>");
    set_equals_var(out, cell, GOTO_TEMP, dataptr);
    *dataptr = cell;
}

// PRE:  
// POST: M[dataptr] == val
void set_equals_nat(FILE *out, int dest, int val, int *dataptr) {
    goto_index(out, dataptr, dest);
    fputs("[-]", out);
    if (val >= 0) {
        for (int i = 0; i < val; i++) fputc('+', out);
    } else {
        for (int i = 0; i < -val; i++) fputc('-', out);
    }
}



// PRE:  
// POST: M[dest] == original M[src], M[src] restored
// Uses TEMP_INDEX as temp
// 1. Sets dest to 0
// 2. Copy src to dest and temp, emptying src
// 3. Copy temp to src, emptying temp
void set_equals_var(FILE *out, int dest, int src, int *dataptr) {
    if (dest == src) return;
    // 1. Sets dest to 0
    set_equals_nat(out, TEMP_INDEX, 0, dataptr);
    set_equals_nat(out, dest, 0, dataptr);
    // 2. Copy src to dest and temp, emptying src
    goto_index(out, dataptr, src);
    fputc('[', out);              // while (M[src] > 0)
    fputc('-', out);              //   M[src]--
    goto_index(out, dataptr, dest);
    fputc('+', out);              //   M[dest]++
    goto_index(out, dataptr, TEMP_INDEX);
    fputc('+', out);              //   M[temp]++
    goto_index(out, dataptr, src);
    fputc(']', out);
    // 3. Copy temp to src, emptying temp
    goto_index(out, dataptr, TEMP_INDEX);
    fputc('[', out);              // while (M[temp] > 0)
    fputc('-', out);              //   M[temp]--
    goto_index(out, dataptr, src);
    fputc('+', out);              //   M[src]++
    goto_index(out, dataptr, TEMP_INDEX);
    fputc(']', out);
}


// sets M[dest] = M[M[cell]]
void set_equals_var_cell(FILE *out, int dest, int cell, int *dataptr) {
    set_equals_nat(out, TEMP_INDEX, 0, dataptr);
    set_equals_nat(out, dest, 0, dataptr);
    // 2. Copy src to dest and temp, emptying src
    goto_index_cell(out, dataptr, cell);
    fputc('[', out);              // while (M[M[cell]] > 0)
    fputc('-', out);              //   M[M[cell]]--
    goto_index(out, dataptr, dest);
    fputc('+', out);              //   M[dest]++
    goto_index(out, dataptr, TEMP_INDEX);
    fputc('+', out);              //   M[temp]++
    goto_index_cell(out, dataptr, cell);
    fputc(']', out);
    // 3. Copy temp to src, emptying temp
    goto_index(out, dataptr, TEMP_INDEX);
    fputc('[', out);              // while (M[temp] > 0)
    fputc('-', out);              //   M[temp]--
    goto_index_cell(out, dataptr, cell);
    fputc('+', out);              //   M[M[cell]]++
    goto_index(out, dataptr, TEMP_INDEX);
    fputc(']', out);
}


// PRE:  M[TEMP_INDEX] == 0
// POST: M[dest] == original M[src], M[src] restored, 
//       M[TEMP_INDEX] == 0
void set_equals(FILE *out, int dest, C_Atom *src_atom, int *dataptr) {
    if (src_atom->type == C_IDENT) {
        int src;
        if (src_atom->offset == NULL) {
            src = varToPos(src_atom->string);
            set_equals_var(out, dest, src, dataptr);
        } else {

            src = varToPos(src_atom->string);
            // M[RESULT_INDEX] <- <expr>
            emit_expr(out, src_atom->offset, dataptr);
            // M[OFFSET_INDEX] <- src
            set_equals_var(out, OFFSET_INDEX, src, dataptr);
            // M[OFFSET_INDEX] <- M[OFFSET_INDEX] + M[RESULT_INDEX]; 
            // M[RESULT_INDEX] <- 0
            emit_plus(out, OFFSET_INDEX, RESULT_INDEX, dataptr);
            goto_index_cell(out, dataptr, OFFSET_INDEX);
        }   
    } else if (src_atom->type == C_VALUE) {
        set_equals_nat(out, dest, src_atom->val, dataptr);
    }
}





// PRE: the two items being compared are at X and Y
// temp0[-]temp1[-]z[-]
// x[ temp0+
//        y[- temp0[-] temp1+ y]
//    temp0[- z+ temp0]
//    temp1[- y+ temp1]
//    y- x- ]
void emit_greater_than(FILE *out, int *dataptr) {
    set_equals_nat(out, TEMP0, 0, dataptr);
    set_equals_nat(out, TEMP1, 0, dataptr);
    set_equals_nat(out, Z, 0, dataptr);

    goto_index(out, dataptr, X);
    fputc('[', out);
        goto_index(out, dataptr, TEMP0);
        fputc('+', out);
        goto_index(out, dataptr, Y);
        fprintf(out, "[-");
            goto_index(out, dataptr, TEMP0);
                fprintf(out, "[-]");
            goto_index(out, dataptr, TEMP1);
            fputc('+', out);
            goto_index(out, dataptr, Y);
        fputc(']', out);
        goto_index(out, dataptr, TEMP0);
        fprintf(out, "[-");
            goto_index(out, dataptr, Z);
            fputc('+', out);
            goto_index(out, dataptr, TEMP0);
        fputc(']', out);
        goto_index(out, dataptr, TEMP1);
        fprintf(out, "[-");
            goto_index(out, dataptr, Y);
            fputc('+', out);
            goto_index(out, dataptr, TEMP1);
        fputc(']', out);
        goto_index(out, dataptr, Y);
        fputc('-', out);
        goto_index(out, dataptr, X);
        fputc('-', out);
    fputc(']', out);
}


// PRE: x is at X, y is at Y
void emit_equals(FILE *out, int *dataptr) {
    set_equals_nat(out, TEMP0, 1, dataptr);
    set_equals_nat(out, RESULT_INDEX, 0, dataptr);
    goto_index(out, dataptr, X);
    fputc('[', out);
        fputc('-', out);
        goto_index(out, dataptr, Y);
        fputc('-', out);
        goto_index(out, dataptr, X);
    fputc(']', out);
    // THIS LOOP SHOULD NEVER RUN, checks if X = 0
    fputc('[', out);
        goto_index(out, dataptr, TEMP0);
        set_equals_nat(out, TEMP0, 0, dataptr);
        goto_index(out, dataptr, X);
        set_equals_nat(out, X, 0, dataptr);
    fputc(']', out);
    goto_index(out, dataptr, Y);
    // checks if Y = 0, if not, X != Y
    fputc('[', out);
        goto_index(out, dataptr, TEMP0);
        set_equals_nat(out, TEMP0, 0, dataptr);
        goto_index(out, dataptr, Y);
        set_equals_nat(out, Y, 0, dataptr);
    fputc(']', out);
    goto_index(out, dataptr, TEMP0);
    fputc('[', out);
        goto_index(out, dataptr, RESULT_INDEX);
        fputc('+', out);
        goto_index(out, dataptr, TEMP0);
        fputc('-', out);
    fputc(']', out);
    goto_index(out, dataptr, RESULT_INDEX);
}

// PRE: NOT_X is the value to be notted
// POST: M[RESULT_INDEX] == !NOT_X
// y <- 1
// x [y - x [-]] 
// x <- y
// res <- x
void emit_not_helper(FILE *out, int *dataptr) {
    set_equals_nat(out, NOT_Y, 1, dataptr);
    goto_index(out, dataptr, NOT_X);
    fputc('[', out);
        goto_index(out, dataptr, NOT_Y);
        fputc('-', out);
        goto_index(out, dataptr, NOT_X);
        fprintf(out, "[-]");
    fputc(']', out);
    set_equals_var(out, NOT_X, NOT_Y, dataptr);
    set_equals_var(out, RESULT_INDEX, NOT_X, dataptr);
    set_equals_nat(out, NOT_X, 0, dataptr);
    set_equals_nat(out, NOT_Y, 0, dataptr);
}


// POST: M[RESULT_INDEX] == !M[dest]
// emit_not_cell directly reaches for the flag instead of taking in an atom
void emit_notflag(FILE *out, int dest, int *dataptr) {
    set_equals_var(out, NOT_X, dest, dataptr);
    emit_not_helper(out, dataptr);

}

// PRE: ATOM = M[IDENT] | NAT
// POST: M[RESULT_INDEX] == !IDENT | M[RESULT_INDEX] == !NAT
void emit_not(FILE *out, C_Atom *atom, int *dataptr) {
    set_equals(out, NOT_X, atom, dataptr);
    emit_not_helper(out, dataptr);
}

// PRE: r <- x, t <- y
// POST: r == x + y, t == 0
void emit_plus(FILE *out, int r, int t, int *dataptr) {
    goto_index(out, dataptr, t);
    fprintf(out, "[-");
        goto_index(out, dataptr, r);
        fputc('+', out);
        goto_index(out, dataptr, t);
    fputc(']', out);
    goto_index(out, dataptr, r);
}

// PRE: M[LHS_INDEX] and M[RHS_INDEX] == 0
// POST: dataptr is at RESULT_INDEX
void emit_expr(FILE *out, C_Expr *expr, int *dataptr) {
    set_equals_nat(out, RESULT_INDEX, 0, dataptr);
    switch(expr->type) {
        case C_EXPR_EQUALS:
            set_equals(out, X, expr->atom1, dataptr);
            set_equals(out, Y, expr->atom2, dataptr);

            emit_equals(out, dataptr);
            break;
        case C_EXPR_GREATER:
            set_equals(out, X, expr->atom1, dataptr);
            set_equals(out, Y, expr->atom2, dataptr);
            
            emit_greater_than(out, dataptr);
            goto_index(out, dataptr, RESULT_INDEX);
            break;
        case C_EXPR_SMALLER:
        // Just C_EXPR_GREATER but swapped
            set_equals(out, X, expr->atom2, dataptr);
            set_equals(out, Y, expr->atom1, dataptr);

            emit_greater_than(out, dataptr);
            goto_index(out, dataptr, RESULT_INDEX);
            break;
        case C_EXPR_PLUS:
        // IN PSEUDO-BF:
        // r, t <- 0
        // r <- a
        // t <- b
        // t [- r + t]
            set_equals_nat(out, RESULT_INDEX, 0, dataptr);
            set_equals_nat(out, EXPR_TEMP_INDEX, 0, dataptr);
            set_equals(out, RESULT_INDEX, expr->atom1, dataptr);
            set_equals(out, EXPR_TEMP_INDEX, expr->atom2, dataptr);
            
            emit_plus(out, RESULT_INDEX, EXPR_TEMP_INDEX, dataptr);
            break;
        case C_EXPR_NOT:
            emit_not(out, expr->atom1, dataptr);
            goto_index(out, dataptr, RESULT_INDEX);
            break; 
        case C_EXPR_SINGLE:
            if ( expr->atom1->type == C_GETCHAR) { // x = getchar();
                goto_index(out, dataptr, RESULT_INDEX);
                fputc(',', out);
            } else {
                set_equals(out, RESULT_INDEX, expr->atom1, dataptr);
            }
            break;
        case C_EXPR_MINUS:
            set_equals_nat(out, RESULT_INDEX, 0, dataptr);
            set_equals_nat(out, EXPR_TEMP_INDEX, 0, dataptr);
            set_equals(out, RESULT_INDEX, expr->atom1, dataptr);
            set_equals(out, EXPR_TEMP_INDEX, expr->atom2, dataptr);
            goto_index(out, dataptr, EXPR_TEMP_INDEX);
            fprintf(out, "[-");
                goto_index(out, dataptr, RESULT_INDEX);
                fputc('-', out);
                goto_index(out, dataptr, EXPR_TEMP_INDEX);
            fputc(']', out);
            goto_index(out, dataptr, RESULT_INDEX);
            break;
        case C_EXPR_INC:
            goto_index(out, dataptr, varToPos(expr->atom1->string));
            fputc('+', out);
            break;
        case C_EXPR_DEC:
            goto_index(out, dataptr, varToPos(expr->atom1->string));
            fputc('-', out);
            break;
        case C_EXPR_MULT:
            // x and y are working copies of atom1 and atom2
            // temp1 <- y
            // x[-
                // r <- r + y 
                // restore y 
                // x
            // ]
            set_equals(out, X, expr->atom1, dataptr);
            set_equals(out, TEMP1, expr->atom2, dataptr);
            set_equals(out, Y, expr->atom2, dataptr);
            goto_index(out, dataptr, X);

            fputc('[', out);
                fputc('-', out);
                emit_plus(out, RESULT_INDEX, Y, dataptr);
                set_equals_var(out, Y, TEMP1, dataptr);
                goto_index(out, dataptr, X);
            fputc(']', out);
            break;
        default:
            fail("Expr not recognized in emit_expr");
    }

}

void emit_putchar(FILE *out, C_Atom *atom, int *dataptr) {
    switch (atom->type) {
        case C_VALUE: // immediate value, eg putchar(5);
            set_equals_nat(out, RESULT_INDEX, atom->val, dataptr);
            fputc('.', out);
            break;
        case C_IDENT:
            int index = varToPos(atom->string);
            goto_index(out, dataptr, index);
            fputc('.', out);
            break;
        default:
            fail("Putchar cannot handle this type of atom: ");
    }
}

#define IF_INDEX 3

// emit(out, block, &dataptr) for top level call
void emit_block(FILE *out, C_Block *block, int *dataptr) {
    C_Block *curr = block;

    while (curr != NULL) {
        switch (curr->stmt->type) {
            case C_STMT_DECLARE:
                break;
            case C_STMT_DECLAREARR:
                break;
            case C_STMT_SETEQUALS: // CAN ONLY HANDLE SINGLETONS ON RHS
                int index = varToPos(curr->stmt->operands[0].atom->string);
                goto_index(out, dataptr, index);
                emit_expr(out, curr->stmt->operands[1].expr, dataptr);
                set_equals_var(out, index, RESULT_INDEX, dataptr);
                break;
            case C_STMT_IF:
                emit_expr(out, curr->stmt->operands[0].expr, dataptr);
                // if M[IF_INDEX] == 0: block is skipped
                // if M[IF_INDEX] != 0: block is run 
                goto_index(out, dataptr, IF_INDEX);
                fputc('[', out);
                emit_block(out, curr->stmt->operands[1].block, dataptr);
                goto_index(out, dataptr, IF_INDEX);
                fprintf(out, "[-]]");
                break;
            case C_STMT_IFELSE:
                emit_expr(out, curr->stmt->operands[0].expr, dataptr);
                // if M[IF_INDEX] == 0: block is skipped
                // if M[IF_INDEX] != 0: block is run 

                // TRUE BLOCK
                goto_index(out, dataptr, IF_INDEX);
                fputc('[', out);
                emit_block(out, curr->stmt->operands[1].block, dataptr);
                goto_index(out, dataptr, IF_INDEX);
                fprintf(out, "[-]]");
                
                // ELSE BLOCK
                // recomputes the condition, not ideal but see if can optimize
                emit_expr(out, curr->stmt->operands[0].expr, dataptr);
                emit_notflag(out, IF_INDEX, dataptr);
                goto_index(out, dataptr, IF_INDEX);
                fputc('[', out);
                emit_block(out, curr->stmt->operands[2].block, dataptr);
                goto_index(out, dataptr, IF_INDEX);
                fprintf(out, "[-]]");
                break;
            case C_STMT_WHILE:
                emit_expr(out, curr->stmt->operands[0].expr, dataptr);
                goto_index(out, dataptr, RESULT_INDEX);
                fputc('[', out);
                emit_block(out, curr->stmt->operands[1].block, dataptr);
                emit_expr(out, curr->stmt->operands[0].expr, dataptr);
                goto_index(out, dataptr, RESULT_INDEX);
                fputc(']', out);
                break;
            case C_STMT_FOR:
                // evaluate x
                // x[
                //  code
                //  evaluate x again
                //  x
                // ]
                // for (INIT; COND; UPDATE) { body }
                // 
                // 
                // INIT
                // while (cond)  {
                //      BODY;
                //      UPDATE;
                // }      
                if (curr->stmt->operands[0].block != NULL) 
                    emit_block(out, curr->stmt->operands[0].block, dataptr);
                if (curr->stmt->operands[1].expr != NULL)
                    emit_expr(out, curr->stmt->operands[1].expr, dataptr);
                goto_index(out, dataptr, RESULT_INDEX);
                fputc('[', out);
                // body
                emit_block(out, curr->stmt->operands[3].block, dataptr); 
                // update
                emit_block(out, curr->stmt->operands[2].block, dataptr); 
                if (curr->stmt->operands[1].expr != NULL) 
                    emit_expr(out, curr->stmt->operands[1].expr, dataptr);
                goto_index(out, dataptr, RESULT_INDEX);
                fputc(']', out);
                break;
            case C_STMT_PUTCHAR:
                emit_putchar(out, curr->stmt->operands[0].expr->atom1, dataptr);
                break;
            case C_STMT_EXPR:
                emit_expr(out, curr->stmt->operands[0].expr, dataptr);
                break;
            default:
                fail("Stmt not recognized");
        }
        curr = curr->next;
    }
}

void emit(FILE *out, C_Block *block) {
    static int dataptr = 0;
    emit_block(out, block, &dataptr);
}