#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "codegen.h"

#define MAX_FILE_SIZE 10000
#define EXIT_SUCCESS 0
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
        fprintf(stderr, "Could not open bf file\n");
        exit(EXIT_CANNOT_READ_INPUT_FILE);
    }
    size_t n = fread(buf, 1, MAX_FILE_SIZE - 1, f);
    buf[n] = '\0';
    fclose(f);
    return buf;
}

void writeLine(FILE* fOut, char* line) {
    fputs(line, fOut);
    fputc('\n', fOut);
}

int main(int argc, char* argv[]) {
    if (argc < 3 || argc > 4) {
        fprintf(stderr,
                "Usage: %s <input.bf> <output.c> [--mode=char|number]\n",
                argv[0]);
        exit(EXIT_FAIL);
    }

    // parse mode flag
    IOmode mode = CHAR_MODE;
    if (argc == 4) {
        if (strcmp(argv[3], "--mode=number") == 0) {
            mode = NUM_MODE;
        } else if (strcmp(argv[3], "--mode=char") == 0) {
            mode = CHAR_MODE;
        } else {
            fprintf(stderr, "Unknown mode: %s\n", argv[3]);
            exit(EXIT_FAIL);
        }
    }

    char* bfInput = read_file(argv[1]);
    if (!bfInput) {
        fprintf(stderr, "Cannot read input file: %s\n", argv[1]);
        exit(EXIT_CANNOT_READ_INPUT_FILE);
    }

    // Tokenize & optimize
    BF_token* tokens = BF_tokenise(bfInput);
    detect_loop(tokens);
    tokens = optimize_runs(tokens);
    tokens = optimize_zero_loops(tokens);
    BF_print_tokens(tokens);

    tokens = optimize_negations(tokens);
    detect_loop(tokens);
    tokens = optimize_shifts(tokens);
    detect_loop(tokens);
    tokens = optimize_negations(tokens);
    tokens = optimize_mul_loops(tokens);
    detect_loop(tokens);
    BF_print_tokens(tokens);
    tokens = optimize_scan_loops(tokens);
    BF_print_tokens(tokens);

    // Write to output
    FILE* fOut = fopen(argv[2], "w");
    if (fOut == NULL) {
        fprintf(stderr, "Cannot open output file: %s\n", argv[2]);
        exit(EXIT_CANNOT_READ_OUTPUT_FILE);
    }

    emitter(tokens, fOut, mode);

    fclose(fOut);
    free(bfInput);
    return EXIT_SUCCESS;
}
