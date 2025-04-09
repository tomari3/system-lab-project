#ifndef CONSTANTS_H
#define CONSTANTS_H

/* Maximums and Limits */
#define MAX_LINE_LENGTH 81       /* Max source line length (80 + newline) */
#define MAX_LABEL_LENGTH 31      /* Max label length */
#define MAX_OPERANDS 2           /* Max operands per instruction */
#define MEMORY_START_ADDRESS 100 /* Starting address for code */
#define MAX_MEMORY_WORDS 4096    /* Arbitrary max memory size - adjust as needed */
#define WORD_SIZE 24             /* Machine word size in bits */

/* ARE Types */
#define ARE_ABSOLUTE 0b100 /* 4 */
#define ARE_RELOCATABLE 0b010 /* 2 */
#define ARE_EXTERNAL 0b001 /* 1 */

/* Boolean TRUE/FALSE */
#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

/* --- Added Enums --- */

/** Defines the type of a parsed line */
typedef enum {
    LINE_EMPTY,
    LINE_COMMENT,
    LINE_DIRECTIVE,
    LINE_INSTRUCTION,
    LINE_ERROR,
    LINE_UNKNOWN /* Should not happen in final version */
} LineType;

/** Defines the type of directive */
typedef enum {
    DIR_DATA,
    DIR_STRING,
    DIR_ENTRY,
    DIR_EXTERN,
    DIR_UNKNOWN /* For error cases */
} DirectiveType;

/** Defines the assembly instruction opcodes */
typedef enum {
    OP_MOV, OP_CMP, OP_ADD, OP_SUB, OP_LEA,
    OP_CLR, OP_NOT, OP_INC, OP_DEC, OP_JMP,
    OP_BNE, OP_JSR, OP_RED, OP_PRN, OP_RTS,
    OP_STOP,
    OP_UNKNOWN /* For error cases or non-instructions */
} Opcode;

/** Defines the operand addressing modes */
typedef enum {
    ADDR_IMMEDIATE = 0, /* #value */
    ADDR_DIRECT = 1,    /* label */
    ADDR_RELATIVE = 2,  /* &label */
    ADDR_REGISTER = 3,  /* rN */
    ADDR_INVALID,       /* Parsing error */
    ADDR_NONE           /* No operand */
} AddressingMode;

/* --- End Added Enums --- */


/* --- Reserved Words --- */
/* Opcodes */
#define OPCODE_MOV_STR "mov"
#define OPCODE_CMP_STR "cmp"
#define OPCODE_ADD_STR "add"
#define OPCODE_SUB_STR "sub"
#define OPCODE_LEA_STR "lea"
#define OPCODE_CLR_STR "clr"
#define OPCODE_NOT_STR "not"
#define OPCODE_INC_STR "inc"
#define OPCODE_DEC_STR "dec"
#define OPCODE_JMP_STR "jmp"
#define OPCODE_BNE_STR "bne"
#define OPCODE_JSR_STR "jsr"
#define OPCODE_RED_STR "red"
#define OPCODE_PRN_STR "prn"
#define OPCODE_RTS_STR "rts"
#define OPCODE_STP_STR "stop"

/* Directives */
#define DIRECTIVE_DATA_STR ".data"
#define DIRECTIVE_STRING_STR ".string"
#define DIRECTIVE_ENTRY_STR ".entry"
#define DIRECTIVE_EXTERN_STR ".extern"
/* --- Added Macro Directive Strings --- */
#define DIRECTIVE_MCRO_STR "mcro"
#define DIRECTIVE_MCROEND_STR "mcroend"
/* --- End Added Macro Directive Strings --- */

/* Registers */
#define REGISTER_R0_STR "r0"
#define REGISTER_R1_STR "r1"
#define REGISTER_R2_STR "r2"
#define REGISTER_R3_STR "r3"
#define REGISTER_R4_STR "r4"
#define REGISTER_R5_STR "r5"
#define REGISTER_R6_STR "r6"
#define REGISTER_R7_STR "r7"

/* Array of all reserved words for easy checking (e.g., label/macro name validation) */
extern const char *RESERVED_WORDS[];
extern const int RESERVED_WORDS_COUNT;

/* --- End Reserved Words --- */


/* Add other constants as needed */

#endif /* CONSTANTS_H */

