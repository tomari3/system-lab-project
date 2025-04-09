#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include "../include/utils.h"
#include "../include/constants.h"

/* --- Added Reserved Words Array Definition --- */
const char *RESERVED_WORDS[] = {
    OPCODE_MOV, OPCODE_CMP, OPCODE_ADD, OPCODE_SUB, OPCODE_LEA,
    OPCODE_CLR, OPCODE_NOT, OPCODE_INC, OPCODE_DEC, OPCODE_JMP,
    OPCODE_BNE, OPCODE_JSR, OPCODE_RED, OPCODE_PRN, OPCODE_RTS,
    OPCODE_STP, /* Adjusted name */
    DIRECTIVE_DATA, DIRECTIVE_STRING, DIRECTIVE_ENTRY, DIRECTIVE_EXTERN,
    DIRECTIVE_MCRO, DIRECTIVE_MCROEND,
    REGISTER_R0, REGISTER_R1, REGISTER_R2, REGISTER_R3,
    REGISTER_R4, REGISTER_R5, REGISTER_R6, REGISTER_R7
};
const int RESERVED_WORDS_COUNT = sizeof(RESERVED_WORDS) / sizeof(RESERVED_WORDS[0]);
/* --- End Added Reserved Words Array Definition --- */

/**
 * @brief Trims leading and trailing whitespace from a string in-place.
 *
 * @param str The string to trim. Modified in-place.
 * @return Pointer to the modified string (same as input str).
 */
char *trim_whitespace(char *str) {
    char *end;

    /* Trim leading space */
    while (isspace((unsigned char)*str)) str++;

    if (*str == 0) /* All spaces? */
        return str;

    /* Trim trailing space */
    end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--;

    /* Write new null terminator character */
    end[1] = '\0';

    return str;
}

/**
 * @brief Checks if a string represents a valid integer.
 * Handles optional leading '+' or '-'.
 *
 * @param str The string to check.
 * @return 1 if it's a valid integer representation, 0 otherwise.
 */
int is_integer(const char *str) {
    if (!str || *str == '\0') {
        return 0; /* Empty string is not an integer */
    }

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
            return 0;
        }
        str++;
    }

    return 1;
}

/**
 * @brief Skips leading whitespace in a string.
 *
 * @param str The input string.
 * @return Pointer to the first non-whitespace character in str, or the null terminator.
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
 *
 * @param size The number of bytes to allocate.
 * @return Pointer to the allocated memory. Exits program on failure.
 */
void *safe_malloc(size_t size) {
    void *ptr = malloc(size);
    if (ptr == NULL) {
        fprintf(stderr, "Fatal Error: Memory allocation failed.\n");
        exit(EXIT_FAILURE);
    }
    return ptr;
}

/**
 * @brief Checks if a given word is a reserved keyword.
 *
 * @param word The word to check.
 * @return 1 if it is reserved, 0 otherwise.
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
/* --- End Added Helper Function --- */
/* Add more utility function implementations as needed */

