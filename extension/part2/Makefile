# Compiler and flags
CC      := gcc
CFLAGS  := -Wall -Wextra -std=gnu99 -g

# Sources, objects, target
SRCS    := $(wildcard *.c)
OBJS    := $(SRCS:.c=.o)
TARGET  := c2bf

TRANSLATOR := c2bf
BF2C_SRCS := main.c tokenise.c variable-table.c parser.c
BF2C_HDRS := tokenise.h variable-table.h

# Link objects into the final executable
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

# Compile .c → .o
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up build artifacts
.PHONY: clean
clean:
	rm -f $(OBJS) $(TARGET)
