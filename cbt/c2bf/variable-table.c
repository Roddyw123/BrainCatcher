#include <assert.h>
#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>
#include <string.h>

#include "tokenise.h"
#include "variable-table.h"

// cells 0-9 reserved for temp vars used in "if", "while", "switch", etc
#define INITIALPOS 10

VariableTable varTable = {
    .head = NULL,
    .tail = NULL,
};

static VarNode* createNode(char* str, int pos) {
    VarNode* node = malloc(sizeof(VarNode));
    assert(node != NULL);

    node->next = NULL;
    node->pos = pos;
    node->str = str;

    return node;
}

void freeVarTable(VariableTable varTable) {
    while (varTable.head != NULL) {
        VarNode *nextnode = varTable.head->next;
        free(varTable.head);
        varTable.head = nextnode; 
    }
}

static bool seen(char* str) {
    for (VarNode* curr = varTable.head; curr != NULL; curr = curr->next) {
        if (STREQ(str, curr->str)) {
            return true;
        }
    }
    return false;
}


// THIS IS THE ARRAY REFACTOR
// void createVarTable(C_Block* block) {
//     int pos = INITIALPOS;
//     VarNode* node;
//     for (C_Block* curr = block; curr != NULL; curr = curr->next) {
//         if (curr->stmt->type == C_STMT_DECLARE) {
//             if (seen(curr->stmt->operands[0].atom->string)) continue;
//             node = createNode(curr->stmt->operands[0].atom->string, pos);
//             if (varTable.tail == NULL) {
//                 varTable.head = node;
//                 varTable.tail = node;
//             } else {
//                 varTable.tail->next = node;
//                 varTable.tail = node;
//             }
//             pos += 1;
//         }
//         if (curr->stmt->type == C_STMT_DECLAREARR) {
//             if (seen(curr->stmt->operands[0].atom->string)) continue;
//             for (int i = 0; i < curr->stmt->operands[1].atom->val; i++) {
//                 // size_t len = strlen(curr->stmt->operands[0].atom->string);
//                 // char *arr_index = malloc(sizeof(char) * (len + 2));
//                 // sprintf(arr_index, "%s[%d]", curr->stmt->operands[0].atom->string, i);
//                 node = createNode(curr->stmt->operands[0].atom->string, pos);
//                 if (varTable.tail == NULL) {
//                     varTable.head = node;
//                     varTable.tail = node;
//                 } else {
//                     varTable.tail->next = node;
//                     varTable.tail = node;
//                 }
//                 pos += 1;
//             }
//         }
//     }
// }

void createVarTable(C_Token* token) {
    int pos = INITIALPOS;
    for (C_Token* curr = token; curr != NULL; curr = curr->next) {
        if (curr->type == C_IDENT) {
            if (seen(curr->string)) continue;
            VarNode* node = createNode(curr->string, pos);
            if (varTable.tail == NULL) {
                varTable.head = node;
                varTable.tail = node;
            } else {
                varTable.tail->next = node;
                varTable.tail = node;
            }
            pos++;
        }
    }
}

// given a variable, returns the position in brainfuck array of cells
int varToPos(char* str) {
    for (VarNode* curr = varTable.head; curr != NULL; curr = curr->next) {
        if (STREQ(str, curr->str)) {
            return curr->pos;
        }
    }

    fprintf(stderr, "Variable with name: %s does not exist in table\n", str);
    exit(1);
}

// int arrToPos(char* arr, int offset) {
//     return varToPos(arr) + offset;
// }

void printVarTable() {
    printf("Variable Table: \n");
    for (VarNode* curr = varTable.head; curr != NULL; curr = curr->next) {
        printf("Variable: %s -> %d\n", curr->str, curr->pos);
    }
}