#include "../include/utils.h"
#include <string.h>
#include <stdio.h>

/* Custom implementation of strdup for C90 compatibility */
char* my_strdup(const char* str) {
    char* new_str;
    size_t len;
    
    if (!str) {
        return NULL;
    }
    
    len = strlen(str) + 1;
    new_str = (char*)malloc(len);
    if (new_str) {
        memcpy(new_str, str, len);
    }
    
    return new_str;
}

/* Custom implementation of strtok_r for C90 compatibility */
char* my_strtok(char* str, const char* delim, char** saveptr) {
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

/* Custom implementation of snprintf for C90 compatibility */
void my_snprintf(char* buffer, size_t size, const char* format, const char* str) {
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
