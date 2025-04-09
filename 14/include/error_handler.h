#ifndef ERROR_HANDLER_H
#define ERROR_HANDLER_H

#include "assembler.h"

/* Function prototypes */
void addError(AssemblerContext* context, const char* message);
void printErrors(AssemblerContext* context);
void freeErrorList(ErrorEntry* head);

#endif /* ERROR_HANDLER_H */
