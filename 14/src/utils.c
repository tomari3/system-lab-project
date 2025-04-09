#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include "../include/utils.h"
#include "../include/constants.h" /* Include constants */

/* --- Updated Reserved Words Array Definition --- */
const char *RESERVED_WORDS[] = {
    OPCODE_MOV_STR, OPCODE_CMP_STR, OPCODE_ADD_STR, OPCODE_SUB_STR, OPCODE_LEA_STR,
    OPCODE_CLR_STR, OPCODE_NOT_STR, OPCODE_INC_STR, OPCODE_DEC_STR, OPCODE_JMP_STR,
    OPCODE_BNE_STR, OPCODE_JSR_STR, OPCODE_RED_STR, OPCODE_PRN_STR, OPCODE_RTS_STR,
    OPCODE_STP_STR,
    DIRECTIVE_DATA_STR, DIRECTIVE_STRING_STR, DIRECTIVE_ENTRY_STR, DIRECTIVE_EXTERN_STR,
    /* Added Macro Directives to Reserved List */
    DIRECTIVE_MCRO_STR, DIRECTIVE_MCROEND_STR,
    /* End Added Macro Directives */
    REGISTER_R0_STR, REGISTER_R1_STR, REGISTER_R2_STR, REGISTER_R3_STR,
    REGISTER_R4_STR, REGISTER_R5_STR, REGISTER_R6_STR, REGISTER_R7_STR
};
/* Recalculate count based on actual entries */
const int RESERVED_WORDS_COUNT = sizeof(RESERVED_WORDS) / sizeof(RESERVED_WORDS[0]);
/* --- End Updated Reserved Words Array Definition --- */


/**
 * @brief Trims leading and trailing whitespace from a string in-place.
 */
char *trim_whitespace(char *str) {
    char *end;
    if (!str) return NULL; /* Handle NULL input */

    /* Trim leading space */
    while (isspace((unsigned char)*str)) str++;

    if (*str == 0) /* All spaces? */
        return str;

    /* Trim trailing space */
    end = str + strlen(str) - 1;
    while (end >= str && isspace((unsigned char)*end)) end--; /* Use >= for single char strings */

    /* Write new null terminator character */
    end[1] = '\0';

    return str;
}

/**
 * @brief Checks if a string represents a valid integer.
 */
int is_integer(const char *str) {
    if (!str || *str == '\0') {
        return 0; /* Empty string is not an integer */
    }
    str = skip_whitespace(str); /* Skip leading whitespace */

    /* Skip optional sign */
    if (*str == '+' || *str == '-') {
        str++;
    }

    if (*str == '\0') {
        return 0; /* Sign only is not an integer */
    }

    /* Check if all remaining characters are digits */
    while (*str) {
        if (!isdigit((unsigned char)*str)) {
            /* Allow trailing whitespace */
             const char *end = str;
             while(*end) {
                 if (!isspace((unsigned char)*end)) return 0; /* Non-digit, non-space found */
                 end++;
             }
             break; /* Only trailing whitespace found */
        }
        str++;
    }

    return 1;
}

/**
 * @brief Skips leading whitespace in a string.
 */
const char *skip_whitespace(const char *str) {
    if (!str) return NULL;
    while (*str && isspace((unsigned char)*str)) {
        str++;
    }
    return str;
}

/**
 * @brief Safely allocates memory using malloc, checks for failure, and exits if allocation fails.
 */
void *safe_malloc(size_t size) {
    void *ptr = malloc(size);
    if (ptr == NULL && size > 0) { /* Check size > 0 as malloc(0) might return NULL */
        fprintf(stderr, "Fatal Error: Memory allocation failed (size: %lu).\n", (unsigned long)size);
        exit(EXIT_FAILURE);
    }
    return ptr;
}

/**
 * @brief Checks if a given word is a reserved keyword.
 */
int is_reserved_word(const char *word) {
    int i;
    if (!word) return 0;
    for (i = 0; i < RESERVED_WORDS_COUNT; ++i) {
        if (strcmp(word, RESERVED_WORDS[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

/* Add more utility function implementations as needed */

