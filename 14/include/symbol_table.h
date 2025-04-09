#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include "assembler.h"

/* Function prototypes */
bool addSymbol(AssemblerContext* context, const char* name, int address, bool isExternal, bool isEntry, bool isData);
SymbolEntry* findSymbol(AssemblerContext* context, const char* name);
void updateDataSymbols(AssemblerContext* context, int icf);
void printSymbolTable(AssemblerContext* context);
void freeSymbolTable(SymbolEntry* head);
bool isValidLabel(const char* label);

#endif /* SYMBOL_TABLE_H */
