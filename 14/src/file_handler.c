#include "../include/file_handler.h"
#include "../include/assembler.h"
#include <stdio.h>
#include <string.h>

/* Custom replacement for snprintf for C90 compliance */
static void my_snprintf(char* buffer, size_t size, const char* format, const char* str) {
    size_t len;
    
    if (!buffer || size == 0) {
        return;
    }
    
    sprintf(buffer, format, str);
    len = strlen(buffer);
    
    if (len >= size) {
        buffer[size - 1] = '\0'; /* Ensure null termination */
    }
}

/* Open all necessary files for assembler */
bool openFiles(AssemblerContext* context, const char* filename) {
    char amFilename[MAX_FILENAME_LENGTH];
    char sourceFilename[MAX_FILENAME_LENGTH];
    
    if (!context || !filename) {
        return false;
    }
    
    /* Store source filename */
    strncpy(context->sourceFileName, filename, MAX_FILENAME_LENGTH - 1);
    context->sourceFileName[MAX_FILENAME_LENGTH - 1] = '\0';
    
    /* Open source file (.as) */
    my_snprintf(sourceFilename, MAX_FILENAME_LENGTH, "%s.as", filename);
    context->sourceFile = fopen(sourceFilename, "r");
    
    if (!context->sourceFile) {
        fprintf(stderr, "Error: Could not open source file '%s'\n", sourceFilename);
        return false;
    }
    
    /* Create expanded source file (.am) */
    my_snprintf(amFilename, MAX_FILENAME_LENGTH, "%s.am", filename);
    context->amFile = fopen(amFilename, "w+");
    
    if (!context->amFile) {
        fprintf(stderr, "Error: Could not create output file '%s'\n", amFilename);
        fclose(context->sourceFile);
        context->sourceFile = NULL;
        return false;
    }
    
    return true;
}

/* Create output files after successful assembly */
bool createOutputFiles(AssemblerContext* context) {
    char objectFilename[MAX_FILENAME_LENGTH];
    char entriesFilename[MAX_FILENAME_LENGTH];
    char externalsFilename[MAX_FILENAME_LENGTH];
    SymbolEntry* current;
    bool hasEntries = false;
    
    if (!context) {
        return false;
    }
    
    /* Create object file (.ob) */
    my_snprintf(objectFilename, MAX_FILENAME_LENGTH, "%s.ob", context->sourceFileName);
    context->objectFile = fopen(objectFilename, "w");
    
    if (!context->objectFile) {
        fprintf(stderr, "Error: Could not create object file '%s'\n", objectFilename);
        return false;
    }
    
    /* Check if any entries exist */
    current = context->symbolTable;
    while (current) {
        if (current->isEntry) {
            hasEntries = true;
            break;
        }
        current = current->next;
    }
    
    /* Create entries file (.ent) if needed */
    if (hasEntries) {
        my_snprintf(entriesFilename, MAX_FILENAME_LENGTH, "%s.ent", context->sourceFileName);
        context->entriesFile = fopen(entriesFilename, "w");
        
        if (!context->entriesFile) {
            fprintf(stderr, "Error: Could not create entries file '%s'\n", entriesFilename);
            fclose(context->objectFile);
            context->objectFile = NULL;
            return false;
        }
    }
    
    /* Prepare externals file (.ext) */
    my_snprintf(externalsFilename, MAX_FILENAME_LENGTH, "%s.ext", context->sourceFileName);
    context->externalsFile = fopen(externalsFilename, "w");
    
    if (!context->externalsFile) {
        fprintf(stderr, "Error: Could not create externals file '%s'\n", externalsFilename);
        fclose(context->objectFile);
        context->objectFile = NULL;
        if (context->entriesFile) {
            fclose(context->entriesFile);
            context->entriesFile = NULL;
        }
        return false;
    }
    
    return true;
}

/* Close all open files */
void closeFiles(AssemblerContext* context) {
    if (!context) {
        return;
    }
    
    if (context->sourceFile) {
        fclose(context->sourceFile);
        context->sourceFile = NULL;
    }
    
    if (context->amFile) {
        fclose(context->amFile);
        context->amFile = NULL;
    }
    
    if (context->objectFile) {
        fclose(context->objectFile);
        context->objectFile = NULL;
    }
    
    if (context->entriesFile) {
        fclose(context->entriesFile);
        context->entriesFile = NULL;
    }
    
    if (context->externalsFile) {
        fclose(context->externalsFile);
        context->externalsFile = NULL;
    }
}

/* Write the final output files after successful assembly */
bool writeOutputFiles(AssemblerContext* context) {
    int i;
    SymbolEntry* current;
    long externalsSize;
    char externalsFilename[MAX_FILENAME_LENGTH];
    
    if (!context) {
        return false;
    }
    
    /* Create output files */
    if (!createOutputFiles(context)) {
        return false;
    }
    
    /* Write object file header */
    fprintf(context->objectFile, "    %d %d\n", 
            context->codeImageSize, context->dataImageSize);
    
    /* Write code image */
    for (i = 0; i < context->codeImageSize; i++) {
        Word word = context->codeImage[i];
        unsigned int encodedValue = (word.value << 3) | (word.a << 2) | (word.r << 1) | word.e;
        
        fprintf(context->objectFile, "%07d %06X\n", 
                MEMORY_START + i, encodedValue & 0xFFFFFF); /* 24 bits (6 hex digits) */
    }
    
    /* Write data image */
    for (i = 0; i < context->dataImageSize; i++) {
        Word word = context->dataImage[i];
        unsigned int encodedValue = (word.value << 3) | (word.a << 2) | (word.r << 1) | word.e;
        
        fprintf(context->objectFile, "%07d %06X\n", 
                MEMORY_START + context->codeImageSize + i, encodedValue & 0xFFFFFF);
    }
    
    /* Write entries file */
    if (context->entriesFile) {
        current = context->symbolTable;
        while (current) {
            if (current->isEntry) {
                fprintf(context->entriesFile, "%s %07d\n", 
                        current->name, current->address);
            }
            current = current->next;
        }
    }
    
    /* Check if externals file has any content */
    fseek(context->externalsFile, 0, SEEK_END);
    externalsSize = ftell(context->externalsFile);
    
    /* If no externals were written, delete the file */
    if (externalsSize == 0) {
        fclose(context->externalsFile);
        context->externalsFile = NULL;
        
        my_snprintf(externalsFilename, MAX_FILENAME_LENGTH, "%s.ext", context->sourceFileName);
        remove(externalsFilename);
    }
    
    return true;
}

/* Word to hexadecimal string */
void wordToHex(Word word, char* hexStr) {
    unsigned int encodedValue = (word.value << 3) | (word.a << 2) | (word.r << 1) | word.e;
    sprintf(hexStr, "%06X", encodedValue & 0xFFFFFF);
}
