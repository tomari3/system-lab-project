#include "../include/directive.h"
#include "../include/assembler.h"
#include <string.h>

/* Check if a string is a directive (.data, .string, etc.) */
bool isDirective(const char* str) {
    if (!str || !*str) {
        return false;
    }
    
    /* Check for the dot prefix */
    if (str[0] != '.') {
        return false;
    }
    
    /* Check for known directives */
    if (strcmp(str, ".data") == 0 ||
        strcmp(str, ".string") == 0 ||
        strcmp(str, ".entry") == 0 ||
        strcmp(str, ".extern") == 0) {
        return true;
    }
    
    return false;
}

/* NOTE: The actual implementations of these functions are in first_pass.c */
/* processDataDirective, processStringDirective, processExternDirective */
/* and processEntryDirective is implemented in second_pass.c */
