#include <stdio.h>
#include <stdlib.h>

#include "tokenise.h"
#include "parser.h"
#include "variable-table.h"
#include "emitter.h"

#define EXIT_SUCCESS 0
#define MAX_FILE_SIZE 10000
#define EXIT_FAIL -1
#define EXIT_CANNOT_READ_INPUT_FILE 1
#define EXIT_CANNOT_READ_OUTPUT_FILE 2

// File Input
char* read_file(const char* path) {
    FILE* f = fopen(path, "r");
    if (!f) return NULL;
    char* buf = malloc(MAX_FILE_SIZE);
    if (buf == NULL) {
        fclose(f);
        fprintf(stderr, "Could not open bf file");
        exit(EXIT_CANNOT_READ_INPUT_FILE);
    }
    size_t n = fread(buf, 1, MAX_FILE_SIZE - 1, f);
    buf[n] = '\0';
    fclose(f);
    return buf;
}

// TO TEST: 
// 1. RUN make
// 2. PUT C CODE INSIDE test.c AND RUN THE COMMAND
//
//      ./c2bf  test.c   <any_output_file_name>
//

int main(int argc, char* argv[]) {

    // Todo: refactor to not use duplicate code: cus this is also in c2bf
    if (argc != 3) {
        fprintf(stderr, "Incorrect use of command\n");
        fprintf(stderr, "   ./c2bf <c_file> <bf_file>\n");
        exit(EXIT_FAIL);
    }

    char* teststr = read_file(argv[1]);

    // char* teststr = "char x = 5; char y = 7; char z = 69;      ";

    C_Token* tokens = c_tokenise(teststr);
    // C_print_tokens(tokens);
    free(teststr);

    C_Token* nexttoken;

    // Creates variable to pos table
    // which you can acces by using: varToPos(char* str)
    createVarTable(tokens); 
    // printVarTable();

    C_Block *block = parse_block(tokens, &nexttoken);
    //C_print_block(block);



    FILE *out = fopen(argv[2], "w"); fail_if_null(out);
    emit(out, block);
    
    freeVarTable(varTable);
    C_free_tokens(tokens);
    C_free_block(block);
    fclose(out);

    return EXIT_SUCCESS;
}