#ifndef INSTRUCTION_H
#define INSTRUCTION_H

#include "assembler.h"

/* Function prototypes */
Operation* getOperation(const char* name);
bool isOperation(const char* str);
AddressingMode getAddressingMode(const char* operand);
bool isRegister(const char* str);
int getRegisterNumber(const char* str);

/* Word encoding functions */
Word encodeFirstWord(Operation* op, AddressingMode srcMode, int srcReg, 
                    AddressingMode dstMode, int dstReg);
Word encodeImmediateWord(int value);
Word encodeDirectWord(int address, bool isExternal);
Word encodeRelativeWord(int offset);

#endif /* INSTRUCTION_H */
