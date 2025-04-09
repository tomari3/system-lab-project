#ifndef ENCODING_H
#define ENCODING_H

#include "data_structures.h" /* For Opcode, AddressingMode, SymbolNode */
#include "constants.h"       /* For ARE bits */

/**
 * @brief Encodes the first word of an instruction.
 * Handles opcode, funct, addressing modes, registers, and sets ARE to Absolute (100).
 *
 * @param op The opcode enum value.
 * @param src_mode The addressing mode of the source operand (or ADDR_NONE).
 * @param src_reg The register number of the source operand (0-7, or ignored).
 * @param dest_mode The addressing mode of the destination operand (or ADDR_NONE).
 * @param dest_reg The register number of the destination operand (0-7, or ignored).
 * @return The encoded 24-bit machine word. Returns 0 on error (e.g., unknown opcode).
 */
unsigned int encode_instruction_first_word(Opcode op,
                                           AddressingMode src_mode, int src_reg,
                                           AddressingMode dest_mode, int dest_reg);

/**
 * @brief Encodes an additional information word for an operand.
 * Handles immediate values, direct addresses, or relative distances.
 * Sets the ARE bits appropriately based on the operand context.
 *
 * @param value The immediate value, address, or distance to encode (up to 21 bits).
 * @param symbol The symbol associated with the operand (NULL if immediate or relative).
 * Used to determine if the address is external or relocatable.
 * @param mode The addressing mode of the operand requiring this word.
 * @return The encoded 24-bit machine word including ARE bits.
 */
unsigned int encode_operand_word(int value, SymbolNode *symbol, AddressingMode mode);


/**
 * @brief Encodes a data value (integer or character ASCII) into a 24-bit word.
 * Data words do not have ARE bits according to the spec (assumed absolute).
 *
 * @param value The integer or ASCII value.
 * @return The encoded 24-bit machine word.
 */
unsigned int encode_data_word(int value);


#endif /* ENCODING_H */

