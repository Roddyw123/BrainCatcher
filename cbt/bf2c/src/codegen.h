#ifndef CODEGEN_H
#define CODEGEN_H

#include <stdio.h>
#include "tokenise.h"

#ifdef __cplusplus
extern "C" {
#endif

// PRE: Each L-Bracket has a Matching R-Bracket
// POST: The 'matching' parameter of a matching L-Bracket and R-Bracket
//       are assigned to each other's pointers.
void detect_loop(BF_token *tokens);

// PRE: tokens is not NULL
// POST: Emits C code corresponding to the Brainfuck tokens into out
void emitter(BF_token *tokens, FILE *out, IOmode mode);

BF_token * optimize_runs(BF_token* tokens);

BF_token * optimize_shifts(BF_token * head);

BF_token *optimize_zero_loops(BF_token *head);

BF_token *optimize_negations(BF_token *head);

BF_token * optimize_mul_loops(BF_token * head);

BF_token * optimize_scan_loops(BF_token * head);

#ifdef __cplusplus
}
#endif

#endif // CODEGEN_H
