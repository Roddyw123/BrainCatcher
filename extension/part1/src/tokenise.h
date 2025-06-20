#include <stdbool.h>

typedef enum { CHAR_MODE, NUM_MODE } IOmode;


typedef enum {
    BF_GREATER,
    BF_LESSER,
    BF_PLUS,
    BF_MINUS,
    BF_PERIOD,
    BF_COMMA,
    BF_LEFT_BRACKET,
    BF_RIGHT_BRACKET,
    BF_ADD_N,
    BF_SUB_N,
    BF_SET_ZERO,
    BF_SHIFT_N,
    BF_SET_CONST,
    BF_MULLOOP,
    BF_SCAN_LEFT,
    BF_SCAN_RIGHT
} BF_char;

typedef struct BF_token{
    BF_char command;
    struct BF_token *next;
    struct BF_token *matching; // This is only relevant for Brackets
    int repeat;
    bool live;
    int index;
    int *mul_offsets;
    int *mul_multipliers;
    int mul_count;
} BF_token;

BF_token *BF_tokenise(char *input);
void BF_print_tokens(BF_token *tokens);