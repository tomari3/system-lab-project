#include "../include/first_pass.h"
#include "../include/assembler.h"
#include "../include/instruction.h"
#include "../include/directive.h"
#include <string.h>
#include <ctype.h>
#include <stdio.h>

/* Custom strtok_r implementation for C90 compatibility */
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

/* Remove leading and trailing whitespace */
static void trim(char* str) {
    char* start = str;
    char* end;
    
    if (!str || !*str) return;
    
    /* Skip leading whitespace */
    while(isspace((unsigned char)*start)) start++;
    
    /* All whitespace? */
    if(!*start) {
        *str = '\0';
        return;
    }
    
    /* Move to end */
    end = start + strlen(start) - 1;
    /* Remove trailing whitespace */
    while(end > start && isspace((unsigned char)*end)) end--;
    
    /* Terminate string after end */
    *(end + 1) = '\0';
    
    /* Move string to beginning if needed */
    if(start > str) {
        memmove(str, start, (end - start) + 2);
    }
}

/* Function to parse and process a line of assembly code */
static bool processLine(AssemblerContext* context, char* line);

/* Function to process a directive statement */
static bool processDirective(AssemblerContext* context, char* label, char* directive, char* operands);

/* Function to process an instruction statement */
static bool processInstruction(AssemblerContext* context, char* label, char* operation, char* operands);

/* First pass main function */
bool firstPass(AssemblerContext* context) {
    FILE* amFile;
    char line[MAX_LINE_LENGTH];
    int lineNumber = 0;
    
    if (!context) {
        return false;
    }
    
    amFile = context->amFile;
    if (!amFile) {
        addError(context, "Input file not open for first pass");
        return false;
    }
    
    /* Initialize counters */
    context->IC = MEMORY_START;
    context->DC = 0;
    
    /* Allocate memory for code and data images */
    context->codeImage = (Word*)calloc(MAX_MEMORY_SIZE, sizeof(Word));
    context->dataImage = (Word*)calloc(MAX_MEMORY_SIZE, sizeof(Word));
    
    if (!context->codeImage || !context->dataImage) {
        addError(context, "Memory allocation failed for code/data images");
        return false;
    }
    
    /* Reset file position */
    rewind(amFile);
    
    /* Process the source file line by line */
    while (fgets(line, MAX_LINE_LENGTH, amFile)) {
        lineNumber++;
        context->lineNumber = lineNumber;
        
        /* Process the line */
        if (!processLine(context, line)) {
            /* Error already added to error list */
            context->hasErrors = true;
            /* Continue processing to find more errors */
        }
    }
    
    /* Update data symbols' addresses */
    updateDataSymbols(context, context->IC - MEMORY_START);
    
    /* Set final sizes */
    context->codeImageSize = context->IC - MEMORY_START;
    context->dataImageSize = context->DC;
    
    return !context->hasErrors;
}

/* Process a single line of assembly code */
static bool processLine(AssemblerContext* context, char* line) {
    char lineCopy[MAX_LINE_LENGTH];
    char cleanLine[MAX_LINE_LENGTH];
    char* token;
    char* saveptr = NULL;
    char label[MAX_LABEL_LENGTH] = "";
    char operation[MAX_LABEL_LENGTH] = "";
    char operands[MAX_LINE_LENGTH] = "";
    char* colonPos;
    char* comment;
    
    /* Make a copy of the line to avoid modifying the original */
    strncpy(lineCopy, line, MAX_LINE_LENGTH - 1);
    lineCopy[MAX_LINE_LENGTH - 1] = '\0';
    
    /* Create a clean version for parsing */
    strcpy(cleanLine, lineCopy);
    
    /* Remove comments */
    comment = strchr(cleanLine, ';');
    if (comment) {
        *comment = '\0';
    }
    
    /* Trim whitespace */
    trim(cleanLine);
    
    /* Skip empty lines */
    if (cleanLine[0] == '\0') {
        return true;
    }
    
    /* Check for label (ends with colon) */
    colonPos = strchr(cleanLine, ':');
    if (colonPos) {
        /* Extract label */
        *colonPos = '\0';
        strncpy(label, cleanLine, MAX_LABEL_LENGTH - 1);
        label[MAX_LABEL_LENGTH - 1] = '\0';
        trim(label);
        
        /* Validate label */
        if (!isValidLabel(label)) {
            char errorMsg[300];
            sprintf(errorMsg, "Invalid label name: '%s'", label);
            addError(context, errorMsg);
            return false;
        }
        
        /* Move parsing position past the label */
        token = colonPos + 1;
    } else {
        /* No label, start from the beginning */
        token = cleanLine;
    }
    
    /* Skip whitespace after label or at beginning */
    while (*token && isspace((unsigned char)*token)) token++;
    
    /* Extract operation/directive */
    saveptr = NULL;
    token = my_strtok(token, " \t\n", &saveptr);
    if (!token) {
        /* Line contains only a label, which is valid */
        return true;
    }
    
    strncpy(operation, token, MAX_LABEL_LENGTH - 1);
    operation[MAX_LABEL_LENGTH - 1] = '\0';
    
    /* Extract operands as everything after the operation */
    if (saveptr && *saveptr) {
        strncpy(operands, saveptr, MAX_LINE_LENGTH - 1);
        operands[MAX_LINE_LENGTH - 1] = '\0';
        trim(operands);
    } else {
        operands[0] = '\0'; /* No operands */
    }
    
    /* Process directive or instruction */
    if (operation[0] == '.') {
        return processDirective(context, label, operation, operands);
    } else {
        return processInstruction(context, label, operation, operands);
    }
}

/* Process a directive statement */
static bool processDirective(AssemblerContext* context, char* label, char* directive, char* operands) {
    if (strcmp(directive, ".data") == 0) {
        return processDataDirective(context, label, operands);
    } else if (strcmp(directive, ".string") == 0) {
        return processStringDirective(context, label, operands);
    } else if (strcmp(directive, ".extern") == 0) {
        return processExternDirective(context, operands);
    } else if (strcmp(directive, ".entry") == 0) {
        /* .entry directives are processed in the second pass */
        return true;
    } else {
        char errorMsg[300];
        sprintf(errorMsg, "Unknown directive: '%s'", directive);
        addError(context, errorMsg);
        return false;
    }
}

/* Implementation of the processDataDirective function from directive.h */
bool processDataDirective(AssemblerContext* context, char* label, char* operands) {
    char* token;
    char* saveptr;
    int value;
    char* endptr;
    Word dataWord;
    
    /* Add label to symbol table if present */
    if (label[0] != '\0') {
        if (!addSymbol(context, label, context->DC, false, false, true)) {
            return false;
        }
    }
    
    /* Parse comma-separated values */
    token = my_strtok(operands, ",", &saveptr);
    while (token) {
        /* Skip leading whitespace */
        while (*token && isspace((unsigned char)*token)) token++;
        
        /* Parse the integer value */
        value = strtol(token, &endptr, 10);
        
        /* Check for parsing errors */
        if (*endptr != '\0' && !isspace((unsigned char)*endptr)) {
            char errorMsg[300];
            sprintf(errorMsg, "Invalid numeric value in .data directive: '%s'", token);
            addError(context, errorMsg);
            return false;
        }
        
        /* Encode data word */
        dataWord.value = value & 0x1FFFFF; /* 21 bits */
        dataWord.a = 1; /* Absolute */
        dataWord.r = 0;
        dataWord.e = 0;
        
        /* Add to data image */
        context->dataImage[context->DC++] = dataWord;
        
        /* Get next token */
        token = my_strtok(NULL, ",", &saveptr);
    }
    
    return true;
}

/* Implementation of the processStringDirective function from directive.h */
bool processStringDirective(AssemblerContext* context, char* label, char* operands) {
    char* start;
    char* end;
    int i;
    Word charWord;
    Word nullWord;
    
    /* Add label to symbol table if present */
    if (label[0] != '\0') {
        if (!addSymbol(context, label, context->DC, false, false, true)) {
            return false;
        }
    }
    
    /* Find start and end quotes */
    start = strchr(operands, '"');
    if (!start) {
        addError(context, "Missing opening quote in .string directive");
        return false;
    }
    
    end = strchr(start + 1, '"');
    if (!end) {
        addError(context, "Missing closing quote in .string directive");
        return false;
    }
    
    /* Extract the string content */
    start++; /* Skip opening quote */
    *end = '\0'; /* Terminate at closing quote */
    
    /* Encode each character */
    for (i = 0; start[i] != '\0'; i++) {
        charWord.value = start[i] & 0x1FFFFF; /* ASCII value (21 bits) */
        charWord.a = 1; /* Absolute */
        charWord.r = 0;
        charWord.e = 0;
        
        /* Add to data image */
        context->dataImage[context->DC++] = charWord;
    }
    
    /* Add null terminator */
    nullWord.value = 0;
    nullWord.a = 1; /* Absolute */
    nullWord.r = 0;
    nullWord.e = 0;
    context->dataImage[context->DC++] = nullWord;
    
    return true;
}

/* Implementation of the processExternDirective function from directive.h */
bool processExternDirective(AssemblerContext* context, char* operands) {
    char* token;
    char* saveptr;
    
    /* Extract the symbol name */
    token = my_strtok(operands, " \t", &saveptr);
    if (!token) {
        addError(context, "Missing symbol name in .extern directive");
        return false;
    }
    
    /* Add to symbol table with address 0 (will be resolved by linker) */
    if (!addSymbol(context, token, 0, true, false, false)) {
        return false;
    }
    
    return true;
}

/* Process an instruction statement */
static bool processInstruction(AssemblerContext* context, char* label, char* operation, char* operands) {
    Operation* op;
    char* srcOperand = NULL;
    char* dstOperand = NULL;
    char* saveptr;
    AddressingMode srcMode = -1;
    AddressingMode dstMode = -1;
    int srcReg = 0;
    int dstReg = 0;
    Word firstWord;
    int wordsCount = 1; /* Initial count: first word */
    char errorMsg[300];
    char* opCopy;
    
    /* Get operation details */
    op = getOperation(operation);
    if (!op) {
        sprintf(errorMsg, "Unknown operation: '%s'", operation);
        addError(context, errorMsg);
        return false;
    }
    
    /* Add label to symbol table if present */
    if (label[0] != '\0') {
        if (!addSymbol(context, label, context->IC, false, false, false)) {
            return false;
        }
    }
    
    /* Parse operands based on operation type */
    if (op->type == TWO_OPERANDS) {
        /* Check if operands are present */
        if (!operands || !*operands) {
            sprintf(errorMsg, "Two-operand instruction requires operands: '%s'", operation);
            addError(context, errorMsg);
            return false;
        }
        
        /* Make a copy for strtok */
        opCopy = my_strdup(operands);
        if (!opCopy) {
            addError(context, "Memory allocation failed");
            return false;
        }
        
        /* Split operands by comma */
        saveptr = NULL;
        srcOperand = my_strtok(opCopy, ",", &saveptr);
        dstOperand = my_strtok(NULL, ",", &saveptr);
        
        if (!srcOperand || !dstOperand) {
            free(opCopy);
            addError(context, "Two-operand instruction requires source and destination");
            return false;
        }
        
        /* Trim whitespace */
        trim(srcOperand);
        trim(dstOperand);
        
        /* Determine addressing modes */
        srcMode = getAddressingMode(srcOperand);
        dstMode = getAddressingMode(dstOperand);
        
        /* Get register numbers if applicable */
        if (srcMode == REGISTER) {
            srcReg = getRegisterNumber(srcOperand);
        }
        
        if (dstMode == REGISTER) {
            dstReg = getRegisterNumber(dstOperand);
        }
        
        /* Validate addressing modes for the operation */
        if (op->opcode == 4 && srcMode != DIRECT) { /* lea only supports direct mode for source */
            free(opCopy);
            addError(context, "lea instruction only supports direct addressing for source operand");
            return false;
        }
        
        /* Count extra words needed */
        if (srcMode == IMMEDIATE || srcMode == DIRECT || srcMode == RELATIVE) {
            wordsCount++;
        }
        
        if (dstMode == IMMEDIATE || dstMode == DIRECT || dstMode == RELATIVE) {
            wordsCount++;
        }
        
        free(opCopy);
    } else if (op->type == ONE_OPERAND) {
        /* Check if operands are present */
        if (!operands || !*operands) {
            sprintf(errorMsg, "One-operand instruction requires an operand: '%s'", operation);
            addError(context, errorMsg);
            return false;
        }
        
        /* Just one operand */
        dstOperand = operands;
        trim(dstOperand);
        
        /* Determine addressing mode */
        dstMode = getAddressingMode(dstOperand);
        
        /* Get register number if applicable */
        if (dstMode == REGISTER) {
            dstReg = getRegisterNumber(dstOperand);
        }
        
        /* Count extra words needed */
        if (dstMode == IMMEDIATE || dstMode == DIRECT || dstMode == RELATIVE) {
            wordsCount++;
        }
    } else {
        /* No operands, make sure none are provided */
        if (operands && *operands) {
            sprintf(errorMsg, "No-operand instruction should not have operands: '%s'", operation);
            addError(context, errorMsg);
            return false;
        }
    }
    
    /* Encode first word */
    firstWord = encodeFirstWord(op, srcMode, srcReg, dstMode, dstReg);
    
    /* Add first word to code image */
    context->codeImage[context->IC - MEMORY_START] = firstWord;
    context->IC++;
    
    /* For now, just reserve space for extra words */
    /* The actual encoding of these extra words happens in the second pass */
    context->IC += (wordsCount - 1);
    
    return true;
}
