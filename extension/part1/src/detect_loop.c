#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "tokenise.h"

// PRE: Each L-Bracket has a Matching R-Bracket
// POST: The 'matching' parameter of a matching L-Bracket and R-Bracket are assigned to each others pointers.
void detect_loop(BF_token * tokens) {
    BF_token* tokenStack[1024];
    BF_token* ptr = tokens;
    int si = 0;
    while (ptr != NULL) {
        if (ptr->command == BF_LEFT_BRACKET) {
            tokenStack[si++] = ptr;
        }
        if (ptr->command == BF_RIGHT_BRACKET) {
            if (si == 0) {
                perror("an L-Bracket must have a matching R-Bracket");
                exit(EXIT_FAILURE);
            }
            BF_token* matchingLBracket = tokenStack[--si];
            matchingLBracket->matching = ptr;
            ptr->matching = matchingLBracket;
        }
        ptr = ptr->next;
    }
    if (si != 0) {;
        perror("There is a stray bracket, non-well formed");
        exit(EXIT_FAILURE);
    }
}