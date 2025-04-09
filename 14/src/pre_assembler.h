#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "../include/pre_assembler.h"
#include "../include/data_structures.h"
#include "../include/utils.h"
#include "../include/constants.h"

/* --- Private Helper Function Declarations --- */

/** Frees the memory allocated for a list of macro lines. */
static void free_macro_lines(MacroLineNode *head);

/** Frees the memory allocated for a list of macro definitions. */
static void free_macro_list(MacroNode *head);

/** Finds a macro definition by name in the list. */
static MacroNode* find_macro(MacroNode *head, const char *name);

/** Adds a line to the body of the current macro definition. */
static int add_macro_line(MacroNode *macro, const char *line);

/** Adds a new macro definition node to the list. Returns NULL on failure. */
static MacroNode* add_macro_definition(MacroNode **head, const char *name);

/* --- Public Function Implementation --- */

int run_pre_assembler(const char *input_filename, const char *output_filename) {
    FILE *fp_in = NULL;
    FILE *fp_out = NULL;
    char line[MAX_LINE_LENGTH];
    char line_copy[MAX_LINE_LENGTH]; /* For tokenizing without modifying original */
    int line_number = 0;
    int error_flag = FALSE;
    int is_defining_macro = FALSE;

    MacroNode *macro_list_head = NULL;
    MacroNode *current_macro_definition = NULL;
    MacroNode *found_macro = NULL;

    const char *token1 = NULL;
    const char *token2 = NULL;
    const char *remainder = NULL;

    /* Open input file */
    fp_in = fopen(input_filename, "r");
    if (!fp_in) {
        fprintf(stderr, "Error: Could not open input file '%s' for reading.\n", input_filename);
        return FALSE;
    }

    /* Open output file */
    fp_out = fopen(output_filename, "w");
    if (!fp_out) {
        fprintf(stderr, "Error: Could not open output file '%s' for writing.\n", output_filename);
        fclose(fp_in);
        return FALSE;
    }

    /* Process input file line by line */
    while (fgets(line, sizeof(line), fp_in)) {
        line_number++;
        strcpy(line_copy, line); /* Work with a copy */

        /* Basic tokenization - get first non-whitespace token */
        token1 = skip_whitespace(line_copy);

        /* Check if line is empty or comment */
        if (*token1 == '\0' || *token1 == ';') {
             if (!is_defining_macro) { /* Don't write comments/empty lines from source if defining macro */
                 fputs(line, fp_out);
             } else {
                 /* Store comments/empty lines as part of macro body if desired */
                 /* Current implementation implicitly skips them from macro body */
                 /* To store: call add_macro_line(current_macro_definition, line); */
             }
            continue;
        }

        /* Extract first word */
        char first_word[MAX_LINE_LENGTH];
        sscanf(token1, "%s", first_word);

        if (is_defining_macro) {
            /* Check for macro end */
            if (strcmp(first_word, DIRECTIVE_MCROEND) == 0) {
                /* Validate mcroend line - should have no other tokens */
                remainder = skip_whitespace(token1 + strlen(DIRECTIVE_MCROEND));
                if (*remainder != '\0' && *remainder != '\n') {
                    fprintf(stderr, "Error (Line %d): Extraneous text after '%s'.\n", line_number, DIRECTIVE_MCROEND);
                    error_flag = TRUE;
                }
                is_defining_macro = FALSE;
                current_macro_definition = NULL;
                /* Do not write mcroend line to output */
            } else {
                /* Add line to current macro definition */
                 if (!add_macro_line(current_macro_definition, line)) {
                     /* Error adding line (memory allocation failed) */
                     error_flag = TRUE;
                     /* Further error handling might be needed */
                 }
            }
        } else { /* Not defining a macro */
            /* Check for macro start */
            if (strcmp(first_word, DIRECTIVE_MCRO) == 0) {
                char macro_name[MAX_LABEL_LENGTH + 1]; /* Max label length for macro name */
                token2 = skip_whitespace(token1 + strlen(DIRECTIVE_MCRO));
                if (sscanf(token2, "%s", macro_name) != 1) {
                    fprintf(stderr, "Error (Line %d): Missing macro name after '%s'.\n", line_number, DIRECTIVE_MCRO);
                    error_flag = TRUE;
                } else {
                    /* Validate macro name */
                    if (is_reserved_word(macro_name)) {
                        fprintf(stderr, "Error (Line %d): Macro name '%s' is a reserved word.\n", line_number, macro_name);
                        error_flag = TRUE;
                    } else if (find_macro(macro_list_head, macro_name)) {
                         fprintf(stderr, "Error (Line %d): Macro '%s' redefined.\n", line_number, macro_name);
                         error_flag = TRUE;
                    } else {
                        /* Validate rest of line - should be empty */
                         remainder = skip_whitespace(token2 + strlen(macro_name));
                         if (*remainder != '\0' && *remainder != '\n') {
                            fprintf(stderr, "Error (Line %d): Extraneous text after macro name '%s'.\n", line_number, macro_name);
                            error_flag = TRUE;
                         } else {
                            /* Start defining macro */
                            is_defining_macro = TRUE;
                            current_macro_definition = add_macro_definition(&macro_list_head, macro_name);
                            if (!current_macro_definition) {
                                fprintf(stderr, "Error (Line %d): Failed to allocate memory for macro definition '%s'.\n", line_number, macro_name);
                                error_flag = TRUE;
                                is_defining_macro = FALSE; /* Cannot continue defining */
                            }
                         }
                    }
                }
                /* Do not write mcro line to output */

            } else if ((found_macro = find_macro(macro_list_head, first_word)) != NULL) {
                 /* Macro invocation */
                 remainder = skip_whitespace(token1 + strlen(first_word));
                 if (*remainder != '\0' && *remainder != '\n') {
                     fprintf(stderr, "Error (Line %d): Extraneous text after macro invocation '%s'.\n", line_number, first_word);
                     error_flag = TRUE;
                 } else {
                     /* Expand macro */
                     MacroLineNode *line_node = found_macro->lines_head;
                     while (line_node) {
                         fputs(line_node->line, fp_out);
                         line_node = line_node->next;
                     }
                 }
                 /* Do not write original invocation line */
            } else {
                /* Regular line - copy to output */
                fputs(line, fp_out);
            }
        }
    } /* End while fgets */

    /* Check for unterminated macro definition */
    if (is_defining_macro) {
        fprintf(stderr, "Error: Macro definition started for '%s' but never ended with '%s'.\n",
                current_macro_definition ? current_macro_definition->name : "UNKNOWN", DIRECTIVE_MCROEND);
        error_flag = TRUE;
    }

    /* Cleanup */
    fclose(fp_in);
    fclose(fp_out);
    free_macro_list(macro_list_head);

    /* If errors occurred, potentially delete the output file */
    if (error_flag) {
        fprintf(stderr, "Pre-assembly failed for '%s'. Output file '%s' may be incomplete or invalid.\n", input_filename, output_filename);
        /* remove(output_filename); // Optionally remove the potentially corrupt .am file */
        return FALSE;
    }

    return TRUE; /* Success */
}


/* --- Private Helper Function Implementations --- */

static void free_macro_lines(MacroLineNode *head) {
    MacroLineNode *tmp;
    while (head) {
        tmp = head;
        head = head->next;
        free(tmp->line);
        free(tmp);
    }
}

static void free_macro_list(MacroNode *head) {
    MacroNode *tmp;
    while (head) {
        tmp = head;
        head = head->next;
        free(tmp->name);
        free_macro_lines(tmp->lines_head);
        free(tmp);
    }
}

static MacroNode* find_macro(MacroNode *head, const char *name) {
    while (head) {
        if (strcmp(head->name, name) == 0) {
            return head;
        }
        head = head->next;
    }
    return NULL;
}

static int add_macro_line(MacroNode *macro, const char *line) {
    MacroLineNode *new_line_node;
    if (!macro || !line) return FALSE;

    new_line_node = (MacroLineNode*)safe_malloc(sizeof(MacroLineNode));
    new_line_node->line = (char*)safe_malloc(strlen(line) + 1);
    strcpy(new_line_node->line, line);
    new_line_node->next = NULL;

    if (!macro->lines_head) { /* First line */
        macro->lines_head = new_line_node;
        macro->lines_tail = new_line_node;
    } else { /* Append to end */
        macro->lines_tail->next = new_line_node;
        macro->lines_tail = new_line_node;
    }
    return TRUE;
}

static MacroNode* add_macro_definition(MacroNode **head, const char *name) {
    MacroNode *new_macro = (MacroNode*)safe_malloc(sizeof(MacroNode));
    new_macro->name = (char*)safe_malloc(strlen(name) + 1);
    strcpy(new_macro->name, name);
    new_macro->lines_head = NULL;
    new_macro->lines_tail = NULL;
    new_macro->next = *head; /* Add to beginning of list */
    *head = new_macro;
    return new_macro;
}


