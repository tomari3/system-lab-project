#ifndef DIRECTIVE_H
#define DIRECTIVE_H

#include "assembler.h"

/* Function prototypes */
bool isDirective(const char* str);

/* Directive processing functions */
bool processDataDirective(AssemblerContext* context, char* label, char* operands);
bool processStringDirective(AssemblerContext* context, char* label, char* operands);
bool processExternDirective(AssemblerContext* context, char* operands);
bool processEntryDirective(AssemblerContext* context, char* operands);

#endif /* DIRECTIVE_H */
