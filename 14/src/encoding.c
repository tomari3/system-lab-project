#include <stdio.h> /* For error printing */
#include "../include/encoding.h"
#include "../include/constants.h"
#include "../include/data_structures.h"

/* --- Helper Function to get Funct field --- */
/** Gets the 'funct' value for opcodes that require it. */
static unsigned int get_funct(Opcode op) {
    switch (op) {
        /* Opcode 2 */
        case OP_ADD: return 1;
        case OP_SUB: return 2;
        /* Opcode 5 */
        case OP_CLR: return 1;
        case OP_NOT: return 2;
        case OP_INC: return 3;
        case OP_DEC: return 4;
        /* Opcode 9 */
        case OP_JMP: return 1;
        case OP_BNE: return 2;
        case OP_JSR: return 3;
        /* Others have funct 0 */
        default: return 0;
    }
}

/* --- Public Function Implementations --- */

/** Encodes the first word of an instruction. */
unsigned int encode_instruction_first_word(Opcode op,
                                           AddressingMode src_mode, int src_reg,
                                           AddressingMode dest_mode, int dest_reg)
{
    unsigned int word = 0;
    unsigned int opcode_val = 0;
    unsigned int funct_val = 0;
    unsigned int src_mode_val = (src_mode == ADDR_NONE || src_mode == ADDR_INVALID) ? 0 : (unsigned int)src_mode;
    unsigned int dest_mode_val = (dest_mode == ADDR_NONE || dest_mode == ADDR_INVALID) ? 0 : (unsigned int)dest_mode;
    unsigned int src_reg_val = (src_mode == ADDR_REGISTER) ? (unsigned int)src_reg : 0;
    unsigned int dest_reg_val = (dest_mode == ADDR_REGISTER) ? (unsigned int)dest_reg : 0;

    /* Map Opcode enum to numeric value */
    switch (op) {
        case OP_MOV: opcode_val = 0; break;
        case OP_CMP: opcode_val = 1; break;
        case OP_ADD: case OP_SUB: opcode_val = 2; break;
        case OP_LEA: opcode_val = 4; break;
        case OP_CLR: case OP_NOT: case OP_INC: case OP_DEC: opcode_val = 5; break;
        case OP_JMP: case OP_BNE: case OP_JSR: opcode_val = 9; break;
        case OP_RED: opcode_val = 12; break;
        case OP_PRN: opcode_val = 13; break;
        case OP_RTS: opcode_val = 14; break;
        case OP_STOP: opcode_val = 15; break;
        default:
            fprintf(stderr, "Error: Cannot encode unknown opcode %d\n", op);
            return 0; /* Error */
    }

    funct_val = get_funct(op);

    /* Assemble the word according to PDF format (Page 3) */
    /* | Unused (6) | Opcode (6) | Src Mode (2) | Src Reg (3) | Dest Mode (2) | Dest Reg (3) | Funct (5) | ARE (3) | */
    /* Bit positions: 23 <- -> 0 */

    word |= (opcode_val & 0x3F) << 18;      /* Opcode (bits 18-23) */
    word |= (src_mode_val & 0x03) << 16;    /* Source Mode (bits 16-17) */
    word |= (src_reg_val & 0x07) << 13;     /* Source Register (bits 13-15) */
    word |= (dest_mode_val & 0x03) << 11;   /* Dest Mode (bits 11-12) */
    word |= (dest_reg_val & 0x07) << 8;     /* Dest Register (bits 8-10) */
    word |= (funct_val & 0x1F) << 3;        /* Funct (bits 3-7) */
    word |= (ARE_ABSOLUTE & 0x07);          /* ARE = Absolute (bits 0-2) */

    /* Mask to 24 bits just in case */
    return word & 0xFFFFFF;
}


/** Encodes an additional information word for an operand. */
unsigned int encode_operand_word(int value, SymbolNode *symbol, AddressingMode mode) {
    unsigned int word = 0;
    unsigned int are_bits = 0;

    /* Determine ARE bits */
    if (mode == ADDR_IMMEDIATE || mode == ADDR_RELATIVE) {
        are_bits = ARE_ABSOLUTE;
    } else if (mode == ADDR_DIRECT) {
        if (symbol == NULL) {
            /* This should ideally be caught earlier */
            fprintf(stderr, "Error: Symbol is NULL for direct addressing mode encoding.\n");
            are_bits = ARE_ABSOLUTE; /* Default to absolute on error? */
        } else if (symbol->type == ATTR_EXTERNAL) {
            are_bits = ARE_EXTERNAL;
            value = 0; /* Address of external is 0, linker fills it */
        } else { /* Internal symbol (code or data) */
            are_bits = ARE_RELOCATABLE;
        }
    } else {
         fprintf(stderr, "Error: encode_operand_word called for invalid mode %d\n", mode);
         are_bits = ARE_ABSOLUTE; /* Default */
    }

    /* Value part (21 bits, starting from bit 3) */
    /* Apply 2's complement logic if necessary (especially for relative distance) */
    /* Assuming 'value' is already correctly calculated (e.g., distance for relative) */
    /* Mask value to 21 bits */
    word |= ((unsigned int)value << 3) & 0xFFFFF8; /* Mask: 1111 1111 1111 1111 1111 1000 */

    /* ARE part (3 bits, starting from bit 0) */
    word |= (are_bits & 0x07);

    return word & 0xFFFFFF; /* Ensure 24 bits */
}

/** Encodes a data value (integer or character ASCII) into a 24-bit word. */
unsigned int encode_data_word(int value) {
     /* Data words are just the value, potentially sign-extended if negative */
     /* Mask to 24 bits */
     return ((unsigned int)value) & 0xFFFFFF;
}

