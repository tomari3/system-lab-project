#ifndef UTILS_H
#define UTILS_H

#include <stdio.h> /* For FILE* */

/**
 * @brief Trims leading and trailing whitespace from a string in-place.
 *
 * @param str The string to trim. Modified in-place.
 * @return Pointer to the modified string (same as input str).
 */
char *trim_whitespace(char *str);

/**
 * @brief Checks if a string represents a valid integer.
 * Handles optional leading '+' or '-'.
 *
 * @param str The string to check.
 * @return 1 if it's a valid integer representation, 0 otherwise.
 */
int is_integer(const char *str);

/**
 * @brief Skips leading whitespace in a string.
 *
 * @param str The input string.
 * @return Pointer to the first non-whitespace character in str, or the null terminator.
 */
const char *skip_whitespace(const char *str);

/**
 * @brief Safely allocates memory using malloc, checks for failure, and exits if allocation fails.
 *
 * @param size The number of bytes to allocate.
 * @return Pointer to the allocated memory. Exits program on failure.
 */
void *safe_malloc(size_t size);

/**
 * @brief Checks if a given word is a reserved keyword.
 *
 * @param word The word to check.
 * @return 1 if it is reserved, 0 otherwise.
 */
int is_reserved_word(const char *word);

/* Add more utility function declarations as needed */

#endif /* UTILS_H */

