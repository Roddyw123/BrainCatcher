#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include "tokenise.h"

#define fail_if_null(ptr) if (ptr == NULL){\
    fprintf(stderr, "Failed to malloc\n");\
    exit(1);}

#define fail_unknown(x) fprintf(stderr, "Unknown token %d", x); exit(1)

// PRE: input string is NUL terminated
// POST: r is NULL terminated
BF_token *BF_tokenise(char *input) {
    BF_token *head = malloc(sizeof(BF_token)); fail_if_null(head);
    BF_token *curr = head;
    
    for(int i = 0; input[i] != '\0'; i++) {
        BF_token *next = malloc(sizeof(BF_token)); fail_if_null(next);
        next->matching = NULL;
        next->live = false;
        switch(input[i]) {
            case '>':
                next->command = BF_GREATER;
                break;
            case '<':
                next->command = BF_LESSER;
                break;
            case '+':
                next->command = BF_PLUS;
                break;
            case '-':
                next->command = BF_MINUS;
                break;
            case '.':
                next->command = BF_PERIOD;
                break;
            case ',':
                next->command = BF_COMMA;
                break;
            case '[':
                next->command = BF_LEFT_BRACKET;
                break;
            case ']':
                next->command = BF_RIGHT_BRACKET;
                break;
            default:
                continue;
        }   
        curr->next = next;
        curr = curr->next;
    }
    curr->next = NULL;
    BF_token *result = head->next;
    free(head);
    return result;
}

// PRE: r is NULL terminated
void BF_print_tokens(BF_token *tokens) {
    printf("Tokens:\n");
    for (BF_token *curr = tokens; curr != NULL; curr = curr->next) {
        switch(curr->command) {
            case BF_GREATER:
                printf(">");
                break;
            case BF_LESSER:
                printf("<");
                break;
            case BF_PLUS:
                printf("+");
                break;
            case BF_MINUS:
                printf("-");
                break;
            case BF_PERIOD:
                printf(".");
                break;
            case BF_COMMA:
                printf(",");
                break;
            case BF_LEFT_BRACKET:
                printf("[");
                break;
            case BF_RIGHT_BRACKET:
                printf("]");
                break;
            case BF_ADD_N:
                /* collapsed ++…+ */
                printf("+%d", curr->repeat);
                break;
            case BF_SUB_N:
                /* collapsed ––––– */
                printf("-%d", curr->repeat);
                break;
            case BF_SHIFT_N:
                /* collapsed pointer moves */
                if (curr->repeat > 0)   printf(">%d",  curr->repeat);
                else                    printf("<%d", -curr->repeat);
                break;
            case BF_SET_ZERO:
                /* a clear-cell loop [-] */
                printf("0");
                break;
            case BF_SET_CONST:
                /* constant fold */
                printf("=%d", curr->repeat);
                break;
            case BF_MULLOOP: printf("[-...]"); break;
            case BF_SCAN_LEFT: printf("[<]"); break;
            case BF_SCAN_RIGHT: printf("[>]"); break;
            default:
                fail_unknown(curr->command);
        }
    }
    printf("\n");
}

// PRE: r is NULL terminated
void free_BF_tokens(BF_token *tokens) {
    BF_token *curr = tokens;
    BF_token *next;
    while (curr != NULL) {
        next = curr->next;
        free(curr);
        curr = next;
    }
}

// int main(void){
//     char *string = "<><><><.,.,.,.-++---+-+-+-+-+,.,[][][][][]";
//     BF_token *tokens = BF_tokenise(string);
//     free_BF_tokens(tokens);
// }