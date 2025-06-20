#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "tokenise.h"
#define MAX_CELLS 1024

// This collapses successive increment and decrements into +=, -= syntax
// PRE: tokens is not null
// POST: an altered tokens list with a size smaller than tokens is returned
// This is 100% Functionally Correct
BF_token * optimize_runs(BF_token* tokens) {
    BF_token * dummy = malloc(sizeof *dummy);
    dummy->next = tokens;
    BF_token* prev = dummy;
    BF_token * current = tokens;
    while (current != NULL) {
        if (current->command != BF_MINUS && current->command != BF_PLUS) {
            prev = current;
            current = current->next;
        } else {
            BF_char state = current->command;
            int count = 1;
            BF_token * run_end = current;
            while (run_end->next != NULL && run_end->next->command == state) {
                run_end = run_end->next;
                count ++;
            }
            if (count > 1) {
                BF_token* newTok = malloc(sizeof *newTok);
                newTok->matching = NULL;
                newTok->command = state == BF_PLUS ? BF_ADD_N : BF_SUB_N;
                newTok->repeat = count;
                prev->next = newTok;
                BF_token * boundary = run_end->next;
                newTok->next = run_end->next;
                BF_token* ptr = current;
                BF_token *tmp;
                while (ptr != boundary) {
                    tmp = ptr->next;
                    free(ptr);
                    ptr = tmp;
                }
                prev = newTok;
                current = boundary;
            } else if (count == 1) {
                prev = current;
                current = current->next;
            }

        }
        
    }
    tokens = dummy->next;
    free(dummy);
    return tokens;
}

// Pre: Head is NOT NULL
// Post: Collapse Pointer Shifts into Shift_N TOKENS 
// RLE based shift collapsing
BF_token * optimize_shifts(BF_token * head) {
    BF_token * dummy = malloc(sizeof *dummy);
    dummy->next = head;
    BF_token* prev = dummy;
    BF_token * cur = head;
    while (cur != NULL) {
        if (!(cur->command == BF_LESSER || cur->command == BF_GREATER || cur->command == BF_SHIFT_N)) {
            prev = cur;
            cur = cur->next;
            continue;
        }
        BF_char state = cur->command;
        int netShift = 0;
        BF_token * run_end = cur;
        // Continue until non-valid token is hit
        while (run_end && (run_end->command == BF_LESSER || run_end->command == BF_GREATER || run_end->command == BF_SHIFT_N)) {
            if (run_end->command == BF_SHIFT_N) {
                netShift += run_end->repeat;
            } else {
                netShift += run_end->command == BF_LESSER ? -1 : 1;
            }
            run_end = run_end->next;
        }
        BF_token * boundary = run_end;
        BF_token * p = cur;
        while (p != boundary) {
            BF_token * tmp = p->next;
            free(p);
            p = tmp;
        }
        prev->next = boundary;
        if (netShift != 0) {
            BF_token * newT = malloc(sizeof * newT);
            newT->command = BF_SHIFT_N;
            newT->repeat = netShift;
            newT->matching = NULL;
            newT->next = boundary;
            prev->next = newT;
            prev = newT;
        }

        cur = boundary;
    
    }
    return dummy->next;
}

// Optimization of Zero Loops. Recognizes [-] in loops to replace it with a set 0
// PRE: Head is not null
// POST: an altered tokens list with a size smaller than tokens is returned
BF_token *optimize_zero_loops(BF_token *head) {
    BF_token * dummy = malloc(sizeof *dummy);
    dummy->next = head;
    BF_token* prev = dummy;
    BF_token * cur = head;
    while (cur != NULL) {
        // The [-] is only detected if and only if the next NEXT linked list node is a right bracket (matching)
        if (cur->command == BF_LEFT_BRACKET) {
            if (cur->next->next == cur->matching && cur->next->command == BF_MINUS) {
                BF_token* newNode = malloc(sizeof *newNode);
                assert(newNode != NULL);
                newNode->command = BF_SET_ZERO;
                newNode->matching = NULL;
                prev->next = newNode;
                BF_token * newNext = cur->next->next->next;
                newNode->next = newNext;
                BF_token* tmp1 = cur;
                BF_token* tmp2 = cur->next;
                BF_token* tmp3 = cur->next->next;
                // free(tmp1);
                // free(tmp2);
                // free(tmp3);
                cur = newNode;
                continue;
            }
        }
        prev = cur;
        cur = cur->next;
    }
    BF_token * tmp = dummy->next;
    free(dummy);
    return tmp;
}
// Constant Folding for simple +/- successions
// PRE: head != NULL
// POST: Adjacent +/- or addN/subN tokens are combined or cancelled
BF_token *optimize_negations(BF_token *head) {
    // Dummy to simplify head‐removal logic
    BF_token *dummy = malloc(sizeof *dummy);
    dummy->next    = head;
    BF_token *prev = dummy;
    BF_token *cur  = head;

    while (cur && cur->next) {
        BF_token *nxt = cur->next;

        //  Detect two tokens both “deltas”? (+, -, addN, subN)
        bool curIsDelta = (cur->command == BF_PLUS
                         || cur->command == BF_MINUS
                         || cur->command == BF_ADD_N
                         || cur->command == BF_SUB_N);
        bool nxtIsDelta = (nxt->command == BF_PLUS
                         || nxt->command == BF_MINUS
                         || nxt->command == BF_ADD_N
                         || nxt->command == BF_SUB_N);

        if (curIsDelta && nxtIsDelta) {
            // extract their signed values
            int a = (cur->command == BF_MINUS    ? -1
                    : cur->command == BF_SUB_N ? -cur->repeat
                    : cur->command == BF_ADD_N ?  cur->repeat
                    /* else BF_PLUS */         :  1);
            int b = (nxt->command == BF_MINUS    ? -1
                    : nxt->command == BF_SUB_N ? -nxt->repeat
                    : nxt->command == BF_ADD_N ?  nxt->repeat
                    /* else BF_PLUS */         :  1);

            int sum = a + b;

            // remove both tokens
            BF_token *after = nxt->next;
            free(cur);
            free(nxt);

            if (sum == 0) {
                // they cancel completely
                prev->next = after;
                cur = after;
            } else {
                // create a folded token
                BF_token *t = malloc(sizeof *t);
                t->command = (sum > 0 ? BF_ADD_N : BF_SUB_N);
                t->repeat  = abs(sum);
                t->matching = NULL;
                t->live     = true;
                t->index    = 0;
                t->next     = after;

                // splice it in
                prev->next = t;
                cur = t;
            }
            continue;
        }

        // handle pure pointer‐shifts folding separately
        if (cur->command == BF_SHIFT_N && nxt->command == BF_SHIFT_N) {
            int sum = cur->repeat + nxt->repeat;
            BF_token *after = nxt->next;
            free(cur);
            free(nxt);

            if (sum == 0) {
                prev->next = after;
                cur = after;
            } else {
                BF_token *t = malloc(sizeof *t);
                t->command  = BF_SHIFT_N;
                t->repeat   = sum;
                t->matching = NULL;
                t->live     = true;
                t->index    = 0;
                t->next     = after;

                prev->next = t;
                cur = t;
            }
            continue;
        }

        // NO fold here
        prev = cur;
        cur  = cur->next;
    }

    head = dummy->next;
    free(dummy);
    return head;
}

// Collapse multiplication loops into smaller code. uses a single pass!
// PRE: head is not null
// POST: true
BF_token * optimize_mul_loops(BF_token * head) {
    BF_token * dummy = malloc(sizeof *dummy);
    dummy->next = head;
    BF_token* prev = dummy;
    BF_token * cur = head;
    while (cur != NULL) {
        if (cur->command == BF_LEFT_BRACKET
         && cur->next != NULL
         && (cur->next->command == BF_MINUS
          || (cur->next->command == BF_SUB_N && cur->next->repeat == 1))
         && cur->matching != NULL) 
        {
            BF_token * end = cur->matching;
            // first pass: validate & collect
            BF_token * tmp = cur->next;
            int ptr = 0, decCount = 0;
            bool valid = true;
            // temp storage for offsets/multipliers
            int cap = 16, size = 0;
            int *offs = malloc(cap * sizeof *offs);
            int *muls = malloc(cap * sizeof *muls);
            memset(muls, 0, cap * sizeof *muls);
            while (tmp != end) {
                switch (tmp->command) {
                    case BF_GREATER: 
                        ptr++; 
                        break;
                    case BF_LESSER:  
                        ptr--; 
                        break;
                    case BF_SHIFT_N: 
                        ptr += tmp->repeat; 
                        break;
                    case BF_MINUS:
                        if (ptr == 0) decCount++;
                        else {
                            if (size == cap) {
                                cap *= 2;
                                offs = realloc(offs, cap * sizeof *offs);
                                muls = realloc(muls, cap * sizeof *muls);
                            }
                            offs[size] = ptr;
                            muls[size++] -= 1;
                        }
                        break;
                    case BF_PLUS:
                        if (ptr != 0) {
                            if (size == cap) {
                                cap *= 2;
                                offs = realloc(offs, cap * sizeof *offs);
                                muls = realloc(muls, cap * sizeof *muls);
                            }
                            offs[size] = ptr;
                            muls[size++] += 1;
                        }
                        break;
                    case BF_ADD_N:
                        if (ptr != 0) {
                            if (size == cap) {
                                cap *= 2;
                                offs = realloc(offs, cap * sizeof *offs);
                                muls = realloc(muls, cap * sizeof *muls);
                            }
                            offs[size] = ptr;
                            muls[size++] += tmp->repeat;
                        }
                        break;
                    case BF_SUB_N:
                        if (ptr != 0) {
                            if (size == cap) {
                                cap *= 2;
                                offs = realloc(offs, cap * sizeof *offs);
                                muls = realloc(muls, cap * sizeof *muls);
                            }
                            offs[size] = ptr;
                            muls[size++] -= tmp->repeat;
                        } else {
                            decCount++;
                        }
                        break;

                    // reject any nested loops, I/O, or zero‐loop / set‐const tokens
                    case BF_LEFT_BRACKET:
                    case BF_RIGHT_BRACKET:
                    case BF_PERIOD:
                    case BF_COMMA:
                    case BF_SET_ZERO:
                    case BF_SET_CONST:
                        valid = false;
                        break;
                    default:
                        // other tokens are safe (e.g. BF_SET_CONST handled above)
                        break;
                }
                if (!valid) break;
                tmp = tmp->next;
            }

            if (!valid || ptr != 0 || decCount != 1) {
                free(offs);
                free(muls);
                prev = cur;
                cur  = cur->next;
                continue;
            }

            // splice in new mul‐loop token
            BF_token *after = end->next;
            BF_token * newLoop = malloc(sizeof *newLoop);
            newLoop->command           = BF_MULLOOP;
            newLoop->matching          = NULL;
            newLoop->mul_offsets       = offs;
            newLoop->mul_count         = size;
            newLoop->mul_multipliers   = muls;

            // free old tokens [cur..end]
            for (BF_token *t = cur; t != after; ) {
                BF_token *n = t->next;
                free(t);
                t = n;
            }

            prev->next    = newLoop;
            newLoop->next = after;
            cur           = newLoop;
            continue;
        }
        prev = cur;
        cur  = cur->next;
    }

    BF_token * tmp = dummy->next;
    free(dummy);
    return tmp;
}


// Handle scan loops, collapse [<] and [>] into while one liner!s
// PRE: Head is not NULL
// POST: true
BF_token * optimize_scan_loops(BF_token * head) {
    BF_token * dummy = malloc(sizeof *dummy);
    dummy->next = head;
    BF_token* prev = dummy;
    BF_token * cur = head;
    while (cur != NULL) {
        if (cur->command == BF_LEFT_BRACKET && cur->next != NULL && cur->next->next != NULL &&
            (cur->next->command == BF_GREATER || cur->next->command == BF_LESSER ||
            (cur->next->command == BF_SHIFT_N && (cur->next->repeat == 1 || cur->next->repeat == -1))) && 
            cur->next->next == cur->matching) {
            BF_token * newToken = malloc(sizeof * newToken);
            assert(newToken != NULL);
            if (cur->next->command == BF_SHIFT_N) {
                if (cur->next->repeat == 1) {
                    newToken->command = BF_SCAN_RIGHT;
                } else {
                    newToken->command = BF_SCAN_LEFT;
                }
            } else {
                if (cur->next->command == BF_GREATER) {
                    newToken->command = BF_SCAN_RIGHT;
                } else {
                    newToken->command = BF_SCAN_LEFT;
                }
            }
            newToken->matching = NULL;
            prev->next = newToken;
            BF_token * tmp = cur;
            BF_token * nex = cur->matching->next;
            while (tmp != nex) {
                BF_token * tmp1 = tmp->next;
                free(tmp);
                tmp = tmp1;
            }
            newToken->next = nex;
            cur = newToken;
            continue;
        }
        prev = cur;
        cur = cur->next;
    }
    BF_token * tmp = dummy->next;
    free(dummy);
    return tmp;
}