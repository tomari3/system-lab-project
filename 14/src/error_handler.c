#include "../include/error_handler.h"
#include "../include/assembler.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Add an error to the error list */
void addError(AssemblerContext* context, const char* message) {
    ErrorEntry* newError;
    
    if (!context || !message) {
        return;
    }
    
    /* Create new error entry */
    newError = (ErrorEntry*)malloc(sizeof(ErrorEntry));
    if (!newError) {
        fprintf(stderr, "Error: Memory allocation failed for error list\n");
        return;
    }
    
    /* Initialize the new entry */
    newError->lineNumber = context->lineNumber;
    strncpy(newError->message, message, 255);
    newError->message[255] = '\0';
    
    /* Add to the beginning of the list */
    newError->next = context->errorList;
    context->errorList = newError;
    
    /* Mark that we have errors */
    context->hasErrors = true;
}

/* Print all errors */
void printErrors(AssemblerContext* context) {
    ErrorEntry* current;
    
    if (!context) {
        return;
    }
    
    current = context->errorList;
    while (current) {
        fprintf(stderr, "Error at line %d: %s\n", current->lineNumber, current->message);
        current = current->next;
    }
}

/* Free the error list */
void freeErrorList(ErrorEntry* head) {
    ErrorEntry* current = head;
    ErrorEntry* next;
    
    while (current) {
        next = current->next;
        free(current);
        current = next;
    }
}
