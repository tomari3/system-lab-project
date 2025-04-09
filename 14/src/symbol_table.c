#include "../include/symbol_table.h"
#include "../include/assembler.h"
#include "../include/directive.h"
#include "../include/instruction.h"
#include <string.h>
#include <ctype.h>
#include <stdio.h>

/* Add a new symbol to the symbol table */
bool addSymbol(AssemblerContext* context, const char* name, int address, 
               bool isExternal, bool isEntry, bool isData) {
    SymbolEntry* existing;
    SymbolEntry* newEntry;
    char errorMsg[300];
    
    if (!context || !name || !*name) {
        return false;
    }
    
    /* Check if the symbol already exists */
    existing = findSymbol(context, name);
    if (existing) {
        /* Allow updating entry status */
        if (isEntry && !existing->isExternal) {
            existing->isEntry = true;
            return true;
        }
        
        /* Error: symbol already defined */
        sprintf(errorMsg, "Symbol '%s' already defined at address %d", 
                name, existing->address);
        addError(context, errorMsg);
        return false;
    }
    
    /* Create new symbol entry */
    newEntry = (SymbolEntry*)malloc(sizeof(SymbolEntry));
    if (!newEntry) {
        addError(context, "Memory allocation failed for symbol table");
        return false;
    }
    
    /* Initialize the new entry */
    strncpy(newEntry->name, name, MAX_LABEL_LENGTH - 1);
    newEntry->name[MAX_LABEL_LENGTH - 1] = '\0';
    newEntry->address = address;
    newEntry->isExternal = isExternal;
    newEntry->isEntry = isEntry;
    newEntry->isData = isData;
    
    /* Add to the beginning of the list */
    newEntry->next = context->symbolTable;
    context->symbolTable = newEntry;
    
    return true;
}

/* Find a symbol in the symbol table by name */
SymbolEntry* findSymbol(AssemblerContext* context, const char* name) {
    SymbolEntry* current;
    
    if (!context || !name || !*name) {
        return NULL;
    }
    
    current = context->symbolTable;
    while (current) {
        if (strcmp(current->name, name) == 0) {
            return current;
        }
        current = current->next;
    }
    
    return NULL;
}

/* Update addresses of all data symbols after first pass */
void updateDataSymbols(AssemblerContext* context, int icf) {
    SymbolEntry* current = context->symbolTable;
    
    while (current) {
        if (current->isData) {
            current->address += icf;
        }
        current = current->next;
    }
}

/* Print the symbol table (for debugging) */
void printSymbolTable(AssemblerContext* context) {
    SymbolEntry* current;
    
    if (!context) {
        return;
    }
    
    printf("Symbol Table:\n");
    printf("%-32s %-10s %-10s %-10s %-10s\n", 
           "Name", "Address", "External", "Entry", "Data");
    printf("------------------------------------------------------------\n");
    
    current = context->symbolTable;
    while (current) {
        printf("%-32s %-10d %-10s %-10s %-10s\n",
               current->name,
               current->address,
               current->isExternal ? "Yes" : "No",
               current->isEntry ? "Yes" : "No",
               current->isData ? "Yes" : "No");
        current = current->next;
    }
    printf("\n");
}

/* Free the symbol table */
void freeSymbolTable(SymbolEntry* head) {
    SymbolEntry* current = head;
    SymbolEntry* next;
    
    while (current) {
        next = current->next;
        free(current);
        current = next;
    }
}

/* Check if a label name is valid */
bool isValidLabel(const char* label) {
    int i;
    
    if (!label || !*label) {
        return false;
    }
    
    /* Label must start with a letter */
    if (!isalpha((unsigned char)label[0])) {
        return false;
    }
    
    /* Label must be alphanumeric */
    for (i = 0; label[i] != '\0'; i++) {
        if (!isalnum((unsigned char)label[i]) && label[i] != '_') {
            return false;
        }
    }
    
    /* Label must not be too long */
    if (i > MAX_LABEL_LENGTH - 1) {
        return false;
    }
    
    /* Label must not be a reserved word */
    if (isOperation(label) || isDirective(label) || isRegister(label)) {
        return false;
    }
    
    return true;
}
