# CBT: C↔Brainfuck Transpiler

A source-to-source compiler suite for translating between Brainfuck and C. This project provides two tools:

- **bf2c**: Transpile Brainfuck code into equivalent C programs.
- **c2bf**: Transpile C programs into Brainfuck.

Both tools include a robust set of optimizations and a suite of tests to ensure correctness.

---

## Features

- **Optimizations** for Brainfuck-to-C:

  - Run-length encoding of repeated operations
  - Pointer shift merging
  - Zero-loop detection (`[-]`)
  - Negation and multiplication loops
  - Scan-loop optimization for fast memory scans

- **Bidirectional conversion**:

  - Convert Brainfuck to readable, efficient C code
  - Convert (a subset of) C back to Brainfuck

- **Modes**:

  - I/O in `char` mode (standard Brainfuck semantics)
  - I/O in `number` mode (numeric input/output)

- **Comprehensive test suites**:

  - Classic programs: Hello World variants, ROT13, Bubble Sort, Fibonacci
  - Advanced demos: Universal Turing Machine, RNG, and more

---

## Project Structure

```plaintext
CBT--C-Brainfuck-Transpiler-/
├── LICENSE
├── README.md           # (you are here)
├── cbt/
│   ├── bf2c/           # Brainfuck-to-C transpiler
│   │   ├── src/
│   │   │   ├── main.c
│   │   │   ├── tokenise.c/.h
│   │   │   ├── detect_loop.c
│   │   │   ├── optimization.c
│   │   │   ├── emitter.c/.h
│   │   │   ├── codegen.h
│   │   │   └── out.c
│   │   ├── Makefile
│   │   └── brainfuck-tests/...
│   └── c2bf/           # C-to-Brainfuck transpiler
│       ├── parser.c/.h
│   ├── emitter.c/.h
│   │       ├── Makefile
│   │   └── samples/
```


## Installation

1. Clone this repository:

   ```bash
   git clone https://github.com/Roddyw123/CBT--C-Brainfuck-Transpiler-.git
   cd CBT--C-Brainfuck-Transpiler-
   ```

2. Build both tools:

   ```bash
   cd cbt/bf2c && make
   cd ../c2bf && make
   ```


## Usage

### Brainfuck → C

```bash
# Transpile foo.bf to foo.c using char I/O mode
./cbt/bf2c/bf2c foo.bf foo.c --mode=char

# Compile and run
gcc -O2 foo.c -o foo
./foo [< input.txt]
```

### C → Brainfuck

```bash
# Transpile example.c to example.bf
./cbt/c2bf/c2bf example.c example.bf

# Run in an interpreter (e.g., bf)
bf example.bf
```

---

## Contributing

Contributions welcome! Please open issues or pull requests for bug reports, improvements, or new test cases.

---

## License

This project is released under the MIT License. See [LICENSE](LICENSE) for details.

