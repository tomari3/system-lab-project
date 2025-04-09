#include "../include/instruction.h"
#include "../include/assembler.h"

/* Operations table as defined in the specification */
static Operation operationsTable[] = {
    {"mov", 0, 0, TWO_OPERANDS},
    {"cmp", 1, 0, TWO_OPERANDS},
    {"add", 2, 1, TWO_OPERANDS},
    {"sub", 2, 2, TWO_OPERANDS},
    {"lea", 4, 0, TWO_OPERANDS},
    {"clr", 5, 1, ONE_OPERAND},
    {"not", 5, 2, ONE_OPERAND},
    {"inc", 5, 3, ONE_OPERAND},
    {"dec", 5, 4, ONE_OPERAND},
    {"jmp", 9, 1, ONE_OPERAND},
    {"bne", 9, 2, ONE_OPERAND},
    {"jsr", 9, 3, ONE_OPERAND},
    {"red", 12, 0, ONE_OPERAND},
    {"prn", 13, 0, ONE_OPERAND},
    {"rts", 14, 0, NO_OPERANDS},
    {"stop", 15, 0, NO_OPERANDS},
    {"", 0, 0, NO_OPERANDS}  /* End marker */
};

/* Returns operation details by name, or NULL if not found */
Operation* getOperation(const char* name) {
    int i;
    for (i = 0; operationsTable[i].name[0] != '\0'; i++) {
        if (strcmp(operationsTable[i].name, name) == 0) {
            return &operationsTable[i];
        }
    }
    return NULL;
}

/* Checks if a string is a valid operation */
bool isOperation(const char* str) {
    return getOperation(str) != NULL;
}

/* Determines the addressing mode of an operand */
AddressingMode getAddressingMode(const char* operand) {
    if (!operand || !*operand) {
        return -1;  /* Invalid operand */
    }
    
    /* Immediate addressing (#number) */
    if (operand[0] == '#') {
        return IMMEDIATE;
    }
    
    /* Register addressing (r0-r7) */
    if (isRegister(operand)) {
        return REGISTER;
    }
    
    /* Relative addressing (&label) */
    if (operand[0] == '&') {
        return RELATIVE;
    }
    
    /* Default to direct addressing (label) */
    return DIRECT;
}

/* Checks if a string is a valid register (r0-r7) */
bool isRegister(const char* str) {
    if (!str || !*str) {
        return false;
    }
    
    /* Check if it starts with 'r' followed by a digit 0-7 */
    if (str[0] == 'r' && strlen(str) == 2 &&
        str[1] >= '0' && str[1] <= '7') {
        return true;
    }
    
    return false;
}

/* Gets the register number (0-7) or -1 if invalid */
int getRegisterNumber(const char* str) {
    if (isRegister(str)) {
        return str[1] - '0';
    }
    return -1;
}

/* Encodes the first word of an instruction */
Word encodeFirstWord(Operation* op, AddressingMode srcMode, int srcReg, 
                    AddressingMode dstMode, int dstReg) {
    Word word = {0};
    unsigned int value = 0;
    
    /* Set operation code (bits 18-23) */
    value |= (op->opcode & 0x3F) << 18;
    
    /* If two operands, set source addressing mode (bits 16-17) */
    if (op->type == TWO_OPERANDS) {
        value |= (srcMode & 0x03) << 16;
        /* Set source register (bits 13-15) */
        value |= (srcReg & 0x07) << 13;
    }
    
    /* If one or two operands, set destination addressing mode (bits 11-12) */
    if (op->type == ONE_OPERAND || op->type == TWO_OPERANDS) {
        value |= (dstMode & 0x03) << 11;
        /* Set destination register (bits 8-10) */
        value |= (dstReg & 0x07) << 8;
    }
    
    /* Set function code (bits 3-7) */
    value |= (op->function & 0x1F) << 3;
    
    /* First word always has A=1, R=0, E=0 */
    word.value = value;
    word.a = 1;
    word.r = 0;
    word.e = 0;
    
    return word;
}

/* Encodes the extra word for immediate addressing */
Word encodeImmediateWord(int value) {
    Word word = {0};
    /* Ensure value fits in 21 bits (signed) */
    word.value = value & 0x1FFFFF;
    word.a = 1;  /* Absolute value */
    word.r = 0;
    word.e = 0;
    return word;
}

/* Encodes the extra word for direct addressing */
Word encodeDirectWord(int address, bool isExternal) {
    Word word = {0};
    word.value = address & 0x1FFFFF;
    word.a = 0;
    
    if (isExternal) {
        word.r = 0;
        word.e = 1;  /* External reference */
    } else {
        word.r = 1;  /* Relocatable */
        word.e = 0;
    }
    
    return word;
}

/* Encodes the extra word for relative addressing */
Word encodeRelativeWord(int offset) {
    Word word = {0};
    /* Offset is a signed value */
    word.value = offset & 0x1FFFFF;
    word.a = 1;  /* Absolute value */
    word.r = 0;
    word.e = 0;
    return word;
}
