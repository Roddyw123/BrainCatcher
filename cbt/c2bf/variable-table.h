#include "tokenise.h"

typedef struct VarNode {
    char* str;  // Name of variable
    int pos;        // Position of bf cell
    struct VarNode* next; // Next item in linked list
} VarNode;

typedef struct {
    VarNode* head;
    VarNode* tail;
} VariableTable;

extern VariableTable varTable;

void createVarTable(C_Token* token);
void freeVarTable(VariableTable varTable);
// void createVarTable(C_Block* block);

int varToPos(char* str);

// int arrToPos(char* arr, int offset);

// TESTING PURPOSES USED IN MAIN
void printVarTable();