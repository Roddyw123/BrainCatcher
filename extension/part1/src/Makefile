# Makefile

CC       := gcc
CFLAGS   := -Wall -Wextra -O2

TRANSLATOR := bf2c
SRCS       := main.c tokenise.c detect_loop.c emitter.c optimization.c
HDRS       := tokenise.h codegen.h

BF_SRC := input.bf
BF_C   := output.c

.PHONY: all test clean

all: $(TRANSLATOR)

# Build the BF→C translator
# (only compile .c files; headers are dependencies but not passed to gcc directly)
$(TRANSLATOR): $(SRCS) $(HDRS)
	$(CC) $(CFLAGS) -o $@ $(SRCS)

# Test: translate input.bf → output.c in number mode
test: $(TRANSLATOR)
	@echo "Translating $(BF_SRC) → $(BF_C) (number mode)"
	./$(TRANSLATOR) $(BF_SRC) $(BF_C) --mode=char

clean:
	rm -f $(TRANSLATOR) $(BF_C)
