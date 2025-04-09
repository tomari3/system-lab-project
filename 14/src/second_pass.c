#include "../include/second_pass.h"
#include "../include/assembler.h"
#include "../include/instruction.h"
#include "../include/directive.h"
#include <string.h>

/* Function to parse and process a line during the second pass */
static bool processLineSecondPass(AssemblerContext* context, char* line, int* currentIC);

/* Function to process an instruction statement in second pass */
static bool processInstructionSecondPass(AssemblerContext* context, char* operation, 
                                        char* operands, int* currentIC);

/* Utility function to replace strtok_r for C90 compliance */
static char* my_strtok(char* str, const char* delim, char** saveptr) {
    char* token;
    
    if (str != NULL) {
        *saveptr = str;
    }
    
    /* Skip leading delimiters */
    *saveptr += strspn(*saveptr, delim);
    
    if (**saveptr == '\0') {
        return NULL;
    }
    
    /* Find the end of the token */
    token = *saveptr;
    *saveptr += strcspn(*saveptr, delim);
    
    if (**saveptr != '\0') {
        *(*saveptr)++ = '\0';
    }
    
    return token;
}

/* Second pass main function */
bool secondPass(AssemblerContext* context) {
    FILE* amFile;
    char line[MAX_LINE_LENGTH];
    int lineNumber = 0;
    int currentIC = MEMORY_START;
    
    if (!context) {
        return false;
    }
    
    amFile = context->amFile;
    if (!amFile) {
        addError(context, "Input file not open for second pass");
        return false;
    }
    
    /* Reset file position */
    rewind(amFile);
    
    /* Process the source file line by line */
    while (fgets(line, MAX_LINE_LENGTH, amFile)) {
        lineNumber++;
        context->lineNumber = lineNumber;
        
        /* Process the line for second pass */
        if (!processLineSecondPass(context, line, &currentIC)) {
            /* Error already added to error list */
            context->hasErrors = true;
        }
    }
    
    return !context->hasErrors;
}

/* Process a single line during the second pass */
static bool processLineSecondPass(AssemblerContext* context, char* line, int* currentIC) {
    char lineCopy[MAX_LINE_LENGTH];
    char* token;
    char* saveptr = NULL;
    char operation[MAX_LABEL_LENGTH] = "";
    char operands[MAX_LINE_LENGTH] = "";
    char* colonPos;
    char* end;
    
    /* Make a copy of the line to avoid modifying the original */
    strncpy(lineCopy, line, MAX_LINE_LENGTH - 1);
    lineCopy[MAX_LINE_LENGTH - 1] = '\0';
    
    /* Skip leading whitespace */
    token = lineCopy;
    while (*token && isspace(*token)) token++;
    
    /* Skip empty lines and comments */
    if (*token == '\0' || *token == ';') {
        return true;
    }
    
    /* Skip labels (they were processed in the first pass) */
    colonPos = strchr(token, ':');
    if (colonPos) {
        token = colonPos + 1;
        
        /* Skip whitespace after label */
        while (*token && isspace(*token)) token++;
    }
    
    /* Extract operation/directive */
    token = my_strtok(token, " \t\n", &saveptr);
    if (!token) {
        /* Line contains only a label, which is valid */
        return true;
    }
    
    strncpy(operation, token, MAX_LABEL_LENGTH - 1);
    operation[MAX_LABEL_LENGTH - 1] = '\0';
    
    /* Extract operands as everything after the operation */
    if (saveptr) {
        strncpy(operands, saveptr, MAX_LINE_LENGTH - 1);
        operands[MAX_LINE_LENGTH - 1] = '\0';
        
        /* Trim leading whitespace from operands */
        token = operands;
        while (*token && isspace(*token)) token++;
        memmove(operands, token, strlen(token) + 1);
        
        /* Trim trailing whitespace from operands */
        if (*operands) {
            end = operands + strlen(operands) - 1;
            while (end > operands && isspace(*end)) *end-- = '\0';
        }
    }
    
    /* Process directive or instruction */
    if (operation[0] == '.') {
        /* Process directives */
        if (strcmp(operation, ".entry") == 0) {
            return processEntryDirective(context, operands);
        } else if (strcmp(operation, ".data") == 0 || 
                 strcmp(operation, ".string") == 0 ||
                 strcmp(operation, ".extern") == 0) {
            /* These were handled in the first pass, nothing to do */
            return true;
        } else {
            char errorMsg[300];
            sprintf(errorMsg, "Unknown directive: '%s'", operation);
            addError(context, errorMsg);
            return false;
        }
    } else {
        /* Process instructions */
        return processInstructionSecondPass(context, operation, operands, currentIC);
    }
}

/* Implementation of the processEntryDirective function from directive.h */
bool processEntryDirective(AssemblerContext* context, char* operands) {
    char* token;
    char* saveptr = NULL;
    SymbolEntry* symbol;
    char errorMsg[300];
    
    /* Extract the symbol name */
    token = my_strtok(operands, " \t", &saveptr);
    if (!token) {
        addError(context, "Missing symbol name in .entry directive");
        return false;
    }
    
    /* Find the symbol in the symbol table */
    symbol = findSymbol(context, token);
    if (!symbol) {
        sprintf(errorMsg, "Entry symbol '%s' not defined", token);
        addError(context, errorMsg);
        return false;
    }
    
    /* Mark the symbol as an entry point */
    if (symbol->isExternal) {
        sprintf(errorMsg, "Symbol '%s' cannot be both external and entry", token);
        addError(context, errorMsg);
        return false;
    }
    
    symbol->isEntry = true;
    
    return true;
}

/* Process an instruction statement during the second pass */
static bool processInstructionSecondPass(AssemblerContext* context, char* operation, 
                                       char* operands, int* currentIC) {
    Operation* op;
    char* srcOperand = NULL;
    char* dstOperand = NULL;
    char* saveptr = NULL;
    AddressingMode srcMode = -1;
    AddressingMode dstMode = -1;
    int extraOffset = 1; /* Start after the first word */
    char* valueStr;
    char* endptr;
    int value;
    Word extraWord;
    SymbolEntry* symbol;
    char errorMsg[300];
    char* labelName;
    int offset;
    
    /* Get operation details */
    op = getOperation(operation);
    if (!op) {
        sprintf(errorMsg, "Unknown operation: '%s'", operation);
        addError(context, errorMsg);
        return false;
    }
    
    /* Parse operands based on operation type */
    if (op->type == TWO_OPERANDS) {
        /* Split operands by comma */
        srcOperand = my_strtok(operands, ",", &saveptr);
        dstOperand = my_strtok(NULL, ",", &saveptr);
        
        /* Skip whitespace */
        while (*srcOperand && isspace(*srcOperand)) srcOperand++;
        while (*dstOperand && isspace(*dstOperand)) dstOperand++;
        
        /* Determine addressing modes */
        srcMode = getAddressingMode(srcOperand);
        dstMode = getAddressingMode(dstOperand);
        
        /* Handle source operand extra word if needed */
        if (srcMode == IMMEDIATE) {
            /* Parse immediate value */
            valueStr = srcOperand + 1; /* Skip # */
            value = strtol(valueStr, &endptr, 10);
            
            /* Encode immediate word */
            extraWord = encodeImmediateWord(value);
            context->codeImage[(*currentIC - MEMORY_START) + extraOffset] = extraWord;
            extraOffset++;
        } else if (srcMode == DIRECT) {
            /* Direct addressing - look up symbol */
            symbol = findSymbol(context, srcOperand);
            
            if (!symbol) {
                sprintf(errorMsg, "Undefined symbol: '%s'", srcOperand);
                addError(context, errorMsg);
                return false;
            }
            
            /* Encode direct word */
            extraWord = encodeDirectWord(symbol->address, symbol->isExternal);
            context->codeImage[(*currentIC - MEMORY_START) + extraOffset] = extraWord;
            
            /* Record external reference if needed */
            if (symbol->isExternal && context->externalsFile) {
                fprintf(context->externalsFile, "%s %07d\n", 
                        symbol->name, (*currentIC) + extraOffset);
            }
            
            extraOffset++;
        } else if (srcMode == RELATIVE) {
            /* Not valid for source operand in most instructions */
            addError(context, "Relative addressing not valid for source operand");
            return false;
        }
        
        /* Handle destination operand extra word if needed */
        if (dstMode == IMMEDIATE) {
            /* Parse immediate value */
            valueStr = dstOperand + 1; /* Skip # */
            value = strtol(valueStr, &endptr, 10);
            
            /* Encode immediate word */
            extraWord = encodeImmediateWord(value);
            context->codeImage[(*currentIC - MEMORY_START) + extraOffset] = extraWord;
            extraOffset++;
        } else if (dstMode == DIRECT) {
            /* Direct addressing - look up symbol */
            symbol = findSymbol(context, dstOperand);
            
            if (!symbol) {
                sprintf(errorMsg, "Undefined symbol: '%s'", dstOperand);
                addError(context, errorMsg);
                return false;
            }
            
            /* Encode direct word */
            extraWord = encodeDirectWord(symbol->address, symbol->isExternal);
            context->codeImage[(*currentIC - MEMORY_START) + extraOffset] = extraWord;
            
            /* Record external reference if needed */
            if (symbol->isExternal && context->externalsFile) {
                fprintf(context->externalsFile, "%s %07d\n", 
                        symbol->name, (*currentIC) + extraOffset);
            }
            
            extraOffset++;
        } else if (dstMode == RELATIVE) {
            /* Relative addressing - calculate offset */
            labelName = dstOperand + 1; /* Skip & */
            symbol = findSymbol(context, labelName);
            
            if (!symbol) {
                sprintf(errorMsg, "Undefined symbol: '%s'", labelName);
                addError(context, errorMsg);
                return false;
            }
            
            /* Calculate relative offset */
            offset = symbol->address - (*currentIC + extraOffset);
            
            /* Encode relative word */
            extraWord = encodeRelativeWord(offset);
            context->codeImage[(*currentIC - MEMORY_START) + extraOffset] = extraWord;
            extraOffset++;
        }
    } else if (op->type == ONE_OPERAND) {
        /* Just one operand */
        dstOperand = operands;
        
        /* Skip whitespace */
        while (*dstOperand && isspace(*dstOperand)) dstOperand++;
        
        /* Determine addressing mode */
        dstMode = getAddressingMode(dstOperand);
        
        /* Handle destination operand extra word if needed */
        if (dstMode == IMMEDIATE) {
            /* Parse immediate value */
            valueStr = dstOperand + 1; /* Skip # */
            value = strtol(valueStr, &endptr, 10);
            
            /* Encode immediate word */
            extraWord = encodeImmediateWord(value);
            context->codeImage[(*currentIC - MEMORY_START) + extraOffset] = extraWord;
            extraOffset++;
        } else if (dstMode == DIRECT) {
            /* Direct addressing - look up symbol */
            symbol = findSymbol(context, dstOperand);
            
            if (!symbol) {
                sprintf(errorMsg, "Undefined symbol: '%s'", dstOperand);
                addError(context, errorMsg);
                return false;
            }
            
            /* Encode direct word */
            extraWord = encodeDirectWord(symbol->address, symbol->isExternal);
            context->codeImage[(*currentIC - MEMORY_START) + extraOffset] = extraWord;
            
            /* Record external reference if needed */
            if (symbol->isExternal && context->externalsFile) {
                fprintf(context->externalsFile, "%s %07d\n", 
                        symbol->name, (*currentIC) + extraOffset);
            }
            
            extraOffset++;
        } else if (dstMode == RELATIVE) {
            /* Relative addressing - calculate offset */
            labelName = dstOperand + 1; /* Skip & */
            symbol = findSymbol(context, labelName);
            
            if (!symbol) {
                sprintf(errorMsg, "Undefined symbol: '%s'", labelName);
                addError(context, errorMsg);
                return false;
            }
            
            /* Calculate relative offset */
            offset = symbol->address - (*currentIC + extraOffset);
            
            /* Encode relative word */
            extraWord = encodeRelativeWord(offset);
            context->codeImage[(*currentIC - MEMORY_START) + extraOffset] = extraWord;
            extraOffset++;
        }
    }
    
    /* Update IC */
    *currentIC += extraOffset;
    
    return true;
}
