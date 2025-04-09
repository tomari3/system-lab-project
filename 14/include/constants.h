#ifndef CONSTANTS_H
#define CONSTANTS_H

/* Maximums and Limits */
#define MAX_LINE_LENGTH 81       /* Max source line length (80 + newline) */
#define MAX_LABEL_LENGTH 31      /* Max label length */
#define MEMORY_START_ADDRESS 100 /* Starting address for code */
#define MAX_MEMORY_WORDS 4096    /* Arbitrary max memory size - adjust as needed */
#define WORD_SIZE 24             /* Machine word size in bits */

/* Reserved Keywords - You might expand this later */
/* Opcodes, Directives, Registers */

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

/* Opcodes */
#define OPCODE_MOV "mov"
#define OPCODE_CMP "cmp"
#define OPCODE_ADD "add"
#define OPCODE_SUB "sub"
#define OPCODE_LEA "lea"
#define OPCODE_CLR "clr"
#define OPCODE_NOT "not"
#define OPCODE_INC "inc"
#define OPCODE_DEC "dec"
#define OPCODE_JMP "jmp"
#define OPCODE_BNE "bne"
#define OPCODE_JSR "jsr"
#define OPCODE_RED "red"
#define OPCODE_PRN "prn"
#define OPCODE_RTS "rts"
#define OPCODE_STP "stop" 

/* Directives */
#define DIRECTIVE_DATA ".data"
#define DIRECTIVE_STRING ".string"
#define DIRECTIVE_ENTRY ".entry"
#define DIRECTIVE_EXTERN ".extern"
#define DIRECTIVE_MCRO "mcro"     /* Macro definition start */
#define DIRECTIVE_MCROEND "mcroend" /* Macro definition end */

/* Registers */
#define REGISTER_R0 "r0"
#define REGISTER_R1 "r1"
#define REGISTER_R2 "r2"
#define REGISTER_R3 "r3"
#define REGISTER_R4 "r4"
#define REGISTER_R5 "r5"
#define REGISTER_R6 "r6"
#define REGISTER_R7 "r7"

/* Array of all reserved words for easy checking */
extern const char *RESERVED_WORDS[];
extern const int RESERVED_WORDS_COUNT;

/* --- End Added Reserved Words --- */

/* Add other constants as needed: instruction codes, addressing mode codes, etc. */

#endif /* CONSTANTS_H */

