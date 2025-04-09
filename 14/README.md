ssembler Implementation

This project implements an assembler for a custom assembly language as defined in the specification document. The assembler converts assembly language source code (`.as` files) into machine code (`.ob` files), along with other necessary files for linking and loading.

## Project Structure

The assembler is structured into several components:

```
assembler/
├── include/                 # Header files
│   ├── assembler.h          # Main assembler definitions
│   ├── preprocessor.h       # Macro processing
│   ├── first_pass.h         # First pass implementation
│   ├── second_pass.h        # Second pass implementation
│   ├── symbol_table.h       # Symbol table management
│   ├── instruction.h        # Instruction definitions
│   ├── directive.h          # Directive handling
│   ├── error_handler.h      # Error management
│   └── file_handler.h       # File I/O operations
├── src/                     # Source files
│   ├── main.c               # Entry point
│   ├── assembler.c          # Main assembler logic
│   ├── preprocessor.c       # Macro expansion
│   ├── first_pass.c         # First pass implementation
│   ├── second_pass.c        # Second pass implementation
│   ├── symbol_table.c       # Symbol table operations
│   ├── instruction.c        # Instruction handling
│   ├── directive.c          # Directive processing
│   ├── error_handler.c      # Error management
│   └── file_handler.c       # File I/O operations
├── tests/                   # Test files
│   ├── test_main.c          # Test entry point
│   └── test_cases/          # Test assembly files
└── Makefile                 # Build configuration
```

## Building the Assembler

To build the assembler, simply run:

```bash
make
```

This will create the executable `bin/assembler`.

## Using the Assembler

To assemble one or more assembly files, run:

```bash
bin/assembler file1 [file2 ...]
```

> **Note**: Do not include file extensions in the command-line arguments. The assembler will automatically append `.as` to the input filenames.

## Output Files

For each input file, the assembler produces the following output files:

- `.am` - Expanded source code after macro processing
- `.ob` - Object file containing the machine code
- `.ent` - Entries file (if there are any entries)
- `.ext` - Externals file (if there are any external references)

## Assembly Language Features

The assembler supports:

- 16 different operations (mov, cmp, add, sub, lea, clr, not, inc, dec, jmp, bne, jsr, red, prn, rts, stop)
- 4 addressing modes (immediate, direct, relative, register)
- Directives (.data, .string, .entry, .extern)
- Macros (defined with `mcro` and `mcroend`)

## Example

An example assembly file (`example1.as`) is included in the project, demonstrating various features of the assembly language.

## Error Handling

The assembler performs extensive error checking and reports any issues with line numbers and detailed error messages. If errors are found, no output files are produced.
