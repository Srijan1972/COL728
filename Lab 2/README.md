# Lab 2

C compiler

The C language grammar (c.y and c.l files) have been taken from [here](http://www.quut.com/c/ANSI-C-grammar-y-2011.html)

## Constructs Supported

- The types int, char, bool, float and void have been supported.
- Variables must be declared before values are set to them.
- Constant size arrays and function calls have been supported
- For, while and do-while loops supported.

## Optimizations Done

- Dead Code removal
- Constant Folding

## Dependencies

- Uses `llvm-6.0` and `g++` to compile.

## Running instructions

- To compile the executable run

```bash
make
```

- To compile and generate code for a c file run

```bash
./cc <filename>
```

**Note:** Basic generated code is found in `code_gen.txt` and optimized code is found in `code_opt.txt`