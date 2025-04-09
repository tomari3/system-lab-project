#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

/* Constants */
#define MAX_LINE_LENGTH 81       /* 80 chars + null terminator */
#define MAX_LABEL_LENGTH 32      /* 31 chars + null terminator */
#define MAX_FILENAME_LENGTH 256
#define MEMORY_START 100         /* Starting address in memory */
#define MAX_MEMORY_SIZE 4096     /* Maximum memory size */

/* Word length constants */
#define WORD_SIZE 24             /* 24-bit words */
#define OPCODE_SIZE 6
#define ADDR_MODE_SIZE 2
#define REGISTER_SIZE 3
#define FUNCTION_SIZE 5
#define ARE_SIZE 3

/* Addressing modes */
typedef enum {
    IMMEDIATE = 0,   /* Mode 0: Immediate addressing */
    DIRECT = 1,      /* Mode 1: Direct addressing */
    RELATIVE = 2,    /* Mode 2: Relative addressing */
    REGISTER = 3     /* Mode 3: Register addressing */
} AddressingMode;

/* Word structure for 24-bit machine word */
typedef struct {
    unsigned int value: 21;  /* Bits 3-23 */
    unsigned int a: 1;       /* Absolute bit */
    unsigned int r: 1;       /* Relocatable bit */
    unsigned int e: 1;       /* External bit */
} Word;

/* Instruction types */
typedef enum {
    NO_OPERANDS,     /* Instructions with no operands (e.g., rts, stop) */
    ONE_OPERAND,     /* Instructions with one operand (e.g., clr, jmp) */
    TWO_OPERANDS     /* Instructions with two operands (e.g., mov, add) */
} InstructionType;

/* Operation codes and functions */
typedef struct {
    char name[5];            /* Operation name */
    int opcode;              /* Operation code (decimal) */
    int function;            /* Function code (decimal) */
    InstructionType type;    /* Number of operands */
} Operation;

/* Symbol table entry */
typedef struct SymbolEntry {
    char name[MAX_LABEL_LENGTH];
    int address;
    bool isExternal;
    bool isEntry;
    bool isData;
    struct SymbolEntry* next;
} SymbolEntry;

/* Macro table entry */
typedef struct MacroEntry {
    char name[MAX_LABEL_LENGTH];
    char* content;           /* The macro content (could be multiple lines) */
    struct MacroEntry* next;
} MacroEntry;

/* Error handling structure */
typedef struct ErrorEntry {
    int lineNumber;
    char message[256];
    struct ErrorEntry* next;
} ErrorEntry;

/* Assembler context for tracking state */
typedef struct {
    int IC;                  /* Instruction Counter */
    int DC;                  /* Data Counter */
    int lineNumber;          /* Current line being processed */
    SymbolEntry* symbolTable;
    MacroEntry* macroTable;
    ErrorEntry* errorList;
    Word* codeImage;         /* Array to store instruction words */
    Word* dataImage;         /* Array to store data words */
    int codeImageSize;       /* Number of words in code image */
    int dataImageSize;       /* Number of words in data image */
    bool hasErrors;          /* Flag for any errors during assembly */
    FILE* sourceFile;
    FILE* amFile;            /* Expanded source file (.am) */
    FILE* objectFile;        /* Machine code output (.ob) */
    FILE* externalsFile;     /* External references (.ext) */
    FILE* entriesFile;       /* Entry points (.ent) */
    char sourceFileName[MAX_FILENAME_LENGTH];
} AssemblerContext;

/* Function prototypes */

/* Main assembler functions */
bool assembleFile(const char* filename);

/* Preprocessor functions */
bool preprocess(AssemblerContext* context);

/* First pass functions */
bool firstPass(AssemblerContext* context);

/* Second pass functions */
bool secondPass(AssemblerContext* context);

/* Symbol table functions */
bool addSymbol(AssemblerContext* context, const char* name, int address, bool isExternal, bool isEntry, bool isData);
SymbolEntry* findSymbol(AssemblerContext* context, const char* name);
void freeSymbolTable(SymbolEntry* head);

/* Macro table functions */
bool addMacro(AssemblerContext* context, const char* name, const char* content);
MacroEntry* findMacro(AssemblerContext* context, const char* name);
void freeMacroTable(MacroEntry* head);

/* Error handling functions */
void addError(AssemblerContext* context, const char* message);
void printErrors(AssemblerContext* context);
void freeErrorList(ErrorEntry* head);

/* Utility functions */
bool isValidLabel(const char* label);
bool isOperation(const char* str);
bool isDirective(const char* str);
bool isRegister(const char* str);
int getRegisterNumber(const char* str);
Operation* getOperation(const char* name);
AddressingMode getAddressingMode(const char* operand);

/* File handling functions */
bool openFiles(AssemblerContext* context, const char* filename);
void closeFiles(AssemblerContext* context);
bool writeOutputFiles(AssemblerContext* context);

/* Context management */
AssemblerContext* createContext();
void freeContext(AssemblerContext* context);

#endif /* ASSEMBLER_H */
