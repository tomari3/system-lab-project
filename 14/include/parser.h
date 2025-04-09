#ifndef PARSER_H
#define PARSER_H

#include "data_structures.h" /* For ParsedLine definition */

/**
 * @brief Parses a single line of assembly code (from .am file).
 *
 * Analyzes the line to identify its type (instruction, directive, comment, empty),
 * extracts label (if present), opcode/directive name, and operands.
 * Performs basic syntax validation for labels, opcodes, directives, and operands.
 * Allocates memory for the ParsedLine structure and its contents. The caller is
 * responsible for freeing this memory using free_parsed_line().
 *
 * @param line The null-terminated string containing the line to parse.
 * @param line_num The original line number (for error reporting).
 * @return A pointer to an allocated ParsedLine structure containing the parsed
 * information, or NULL if memory allocation fails. Check the 'has_error'
 * field within the returned structure for parsing errors.
 */
ParsedLine* parse_line(const char *line, int line_num);

/**
 * @brief Frees the memory allocated for a ParsedLine structure and its contents.
 *
 * @param parsed_line A pointer to the ParsedLine structure to free. If NULL,
 * the function does nothing.
 */
void free_parsed_line(ParsedLine *parsed_line);


#endif /* PARSER_H */

