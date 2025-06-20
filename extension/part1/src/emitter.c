#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "codegen.h"

// PRE: tokens is not null, for some arbitrary token -> token->element NOT NULL
// POST: true
void emitter(BF_token* tokens, FILE *out, IOmode mode) {
    int indent = 4;
    BF_token* ptr = tokens;
    fprintf(out, "#include <stdio.h>\n");
    fprintf(out, "#include <stdlib.h>\n");
    fprintf(out, "#include <assert.h>\n");

    fprintf(out, "\nint main() {\n");
    if (mode == NUM_MODE) {
        fprintf(out, "    unsigned int *realPtr = calloc(50000, 8);\n");
    } else {
        fprintf(out, "    unsigned char *realPtr = calloc(50000, 1);\n");
    }
    fprintf(out, "    assert(realPtr != NULL);\n");
    if (mode == NUM_MODE) {
        fprintf(out, "    int *ptr = realPtr;\n");
    } else {
        fprintf(out, "    unsigned char *ptr = realPtr;\n");
    }
    bool mulLoopDetectedAlready = false;
    while (ptr != NULL) {
        switch(ptr->command) {
            case BF_COMMA: {
                if (mode == NUM_MODE) {
                    for (int i = 0; i < indent; i++) fputc(' ', out); fprintf(out, "scanf(\"%%d\", ptr);\n"); break;
                } else {
                    for (int i = 0; i < indent; i++) fputc(' ', out); fprintf(out, "*ptr = (unsigned char)getchar();\n"); break;
                }
            }
            case BF_GREATER: for (int i = 0; i < indent; i++) fputc(' ', out); fprintf(out, "++ptr;\n"); break;
            case BF_LESSER: for (int i = 0; i < indent; i++) fputc(' ', out); fprintf(out, "--ptr;\n"); break;
            case BF_MINUS: for (int i = 0; i < indent; i++) fputc(' ', out); fprintf(out, "--(*ptr);\n"); break;
            case BF_PERIOD: {
                if (mode == NUM_MODE) {
                    for (int i = 0; i < indent; i++) fputc(' ', out); fprintf(out, "printf(\"%%d\\n\", *ptr);\n"); break;
                } else {
                    for (int i = 0; i < indent; i++) fputc(' ', out); fprintf(out, "putchar(*ptr);\n"); break;
                }
            }
            case BF_PLUS: for (int i = 0; i < indent; i++) fputc(' ', out); fprintf(out, "++(*ptr);\n"); break;
            case BF_LEFT_BRACKET:
                for (int i = 0; i < indent; i++) fputc(' ', out);
                fprintf(out, "while (*ptr) {\n");
                indent += 4;
                break;
            case BF_RIGHT_BRACKET:
                indent -= 4;
                for (int i = 0; i < indent; i++) fputc(' ', out);
                fprintf(out, "}\n");
                break;
            case BF_ADD_N:
                for (int i = 0; i < indent; i++) fputc(' ', out);
                fprintf(out, "*ptr += %d;\n", ptr->repeat);
                break;
            case BF_SUB_N:
                for (int i = 0; i < indent; i++) fputc(' ', out);
                fprintf(out, "*ptr -= %d;\n", ptr->repeat);
                break;
            case BF_SET_ZERO:
                for (int i = 0; i < indent; i++) fputc(' ', out);
                fprintf(out, "*ptr = 0;\n");
                break;
            case BF_SHIFT_N:
                for (int i = 0; i < indent; i++) fputc(' ', out);
                if (ptr->repeat == 1) {
                    fprintf(out, "ptr ++;\n");
                } else if (ptr->repeat == -1) {
                    fprintf(out, "ptr --;\n");
                } else if (ptr->repeat > 0) {
                    fprintf(out, "ptr += %d;\n", ptr->repeat);
                } else if (ptr->repeat < 0) {
                    int newRep = -(ptr->repeat);
                    fprintf(out, "ptr -= %d;\n", newRep);
                }
                break;
            case BF_SET_CONST:
                for (int i = 0; i < indent; i++) fputc(' ', out);
                fprintf(out, "*ptr = %d;\n", ptr->repeat);
                break;
            case BF_MULLOOP:
                for (int k = 0; k < indent; k++) fputc(' ', out);
                if (mulLoopDetectedAlready != true) {
                    if (mode == NUM_MODE) {
                        fputs("unsigned int v = *ptr;\n", out);
                    } else {
                        fputs("unsigned char v = *ptr;\n", out);
                    }
                    mulLoopDetectedAlready = true;
                } else {
                    fputs("v = *ptr;\n", out);
                }
                for (int i = 0; i < ptr->mul_count; i++) {
                    for (int j = 0; j < indent; j++) fputc(' ', out);
                    if (ptr->mul_multipliers[i] != 1) {
                        fprintf(out,
                            "*(ptr + %d) += v * %d;\n",
                            ptr->mul_offsets[i],
                            ptr->mul_multipliers[i]
                        );
                    } else {
                        fprintf(out,
                            "*(ptr + %d) += v;\n",
                            ptr->mul_offsets[i],
                            ptr->mul_multipliers[i]
                        );
                    }
                }
                for (int i = 0; i < indent; i++) fputc(' ', out);
                fprintf(out, "*ptr = 0;\n");
                break;
            case BF_SCAN_LEFT: 
                for (int i = 0; i < indent; i++) fputc(' ', out);
                fprintf(out, "while (*ptr) ptr --;\n");
                break;
            case BF_SCAN_RIGHT:
                for (int i = 0; i < indent; i++) fputc(' ', out);
                fprintf(out, "while (*ptr) ptr ++;\n");
                break;
            default: assert(false && "unhandled token");
        }
        ptr = ptr->next;
    }
    for (int i = 0; i < indent; i++) fputc(' ', out);
    fprintf(out, "free(realPtr);\n");
    for (int i = 0; i < indent; i++) fputc(' ', out);
    fprintf(out, "return 0;\n");
    fprintf(out, "}\n");
}