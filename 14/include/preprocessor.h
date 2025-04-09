#ifndef PREPROCESSOR_H
#define PREPROCESSOR_H

#include "assembler.h"

/* Function prototypes */
bool preprocess(AssemblerContext* context);
bool addMacro(AssemblerContext* context, const char* name, const char* content);
MacroEntry* findMacro(AssemblerContext* context, const char* name);
void freeMacroTable(MacroEntry* head);

#endif /* PREPROCESSOR_H */
