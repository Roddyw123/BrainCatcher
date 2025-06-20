#ifndef C_EMITTER_H
#define C_EMITTER_H

#include <stdio.h>
#include "parser.h"


void emit(FILE *out, C_Block *block);
void emit_expr(FILE *out, C_Expr *expr, int *dataptr);
void emit_plus(FILE *out, int r, int t, int *dataptr);
void set_equals_var(FILE *out, int dest, int src, int *dataptr);
#endif