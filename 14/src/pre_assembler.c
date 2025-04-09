#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "../include/pre_assembler.h"
#include "../include/data_structures.h"
#include "../include/utils.h"
#include "../include/constants.h"

/* --- Private Helper Function Declarations --- */
static void free_macro_lines(MacroLineNode *head);
static void free_macro_list(MacroNode *head);
static MacroNode* find_macro(MacroNode *head, const char *name);
static int add_macro_line(MacroNode *macro, const char *line);
static MacroNode* add_macro_definition(MacroNode **head, const char *name);

/* --- Public Function Implementation --- */
int run_pre_assembler(const char *input_filename, const char *output_filename) {
    FILE *fp_in = NULL;
    FILE *fp_out = NULL;
    char line[MAX_LINE_LENGTH];
    char line_copy[MAX_LINE_LENGTH];
    int line_number = 0;
    int error_flag = FALSE;
    int is_defining_macro = FALSE;

    MacroNode *macro_list_head = NULL;
    MacroNode *current_macro_definition = NULL;
    MacroNode *found_macro = NULL;

    const char *token1 = NULL;
    const char *token2 = NULL;
    const char *remainder = NULL;

    /* --- Moved Declarations to Top (Fix for -Wdeclaration-after-statement) --- */
    char first_word[MAX_LINE_LENGTH];
    char macro_name[MAX_LABEL_LENGTH + 1];
    /* --- End Moved Declarations --- */

    fp_in = fopen(input_filename, "r");
    if (!fp_in) {
        fprintf(stderr, "Error: Could not open input file '%s' for reading.\n", input_filename);
        return FALSE;
    }

    fp_out = fopen(output_filename, "w");
    if (!fp_out) {
        fprintf(stderr, "Error: Could not open output file '%s' for writing.\n", output_filename);
        fclose(fp_in);
        return FALSE;
    }

    while (fgets(line, sizeof(line), fp_in)) {
        line_number++;
        strcpy(line_copy, line);
        token1 = skip_whitespace(line_copy);

        if (*token1 == '\0' || *token1 == ';') {
             if (!is_defining_macro) {
                 fputs(line, fp_out);
             }
            continue;
        }

        /* Extract first word */
        if (sscanf(token1, "%s", first_word) != 1) {
             continue;
        }

        if (is_defining_macro) {
             /* Use DIRECTIVE_MCROEND_STR */
            if (strcmp(first_word, DIRECTIVE_MCROEND_STR) == 0) {
                remainder = skip_whitespace(token1 + strlen(DIRECTIVE_MCROEND_STR));
                if (*remainder != '\0' && *remainder != '\n' && !isspace(*remainder)) {
                    fprintf(stderr, "Error (Line %d): Extraneous text after '%s'.\n", line_number, DIRECTIVE_MCROEND_STR);
                    error_flag = TRUE;
                }
                is_defining_macro = FALSE;
                current_macro_definition = NULL;
            } else {
                 if (!add_macro_line(current_macro_definition, line)) {
                     error_flag = TRUE;
                 }
            }
        } else {
            /* Use DIRECTIVE_MCRO_STR */
            if (strcmp(first_word, DIRECTIVE_MCRO_STR) == 0) {
                token2 = skip_whitespace(token1 + strlen(DIRECTIVE_MCRO_STR));
                if (sscanf(token2, "%s", macro_name) != 1) {
                    fprintf(stderr, "Error (Line %d): Missing macro name after '%s'.\n", line_number, DIRECTIVE_MCRO_STR);
                    error_flag = TRUE;
                } else {
                    if (is_reserved_word(macro_name)) {
                        fprintf(stderr, "Error (Line %d): Macro name '%s' is a reserved word.\n", line_number, macro_name);
                        error_flag = TRUE;
                    } else if (find_macro(macro_list_head, macro_name)) {
                         fprintf(stderr, "Error (Line %d): Macro '%s' redefined.\n", line_number, macro_name);
                         error_flag = TRUE;
                    } else {
                         remainder = skip_whitespace(token2 + strlen(macro_name));
                         if (*remainder != '\0' && *remainder != '\n' && !isspace(*remainder)) {
                            fprintf(stderr, "Error (Line %d): Extraneous text after macro name '%s'.\n", line_number, macro_name);
                            error_flag = TRUE;
                         } else {
                            is_defining_macro = TRUE;
                            current_macro_definition = add_macro_definition(&macro_list_head, macro_name);
                            if (!current_macro_definition) {
                                fprintf(stderr, "Error (Line %d): Failed to allocate memory for macro definition '%s'.\n", line_number, macro_name);
                                error_flag = TRUE;
                                is_defining_macro = FALSE;
                            }
                         }
                    }
                }

            } else if ((found_macro = find_macro(macro_list_head, first_word)) != NULL) {
                 remainder = skip_whitespace(token1 + strlen(first_word));
                 if (*remainder != '\0' && *remainder != '\n' && !isspace(*remainder)) {
                     fprintf(stderr, "Error (Line %d): Extraneous text after macro invocation '%s'.\n", line_number, first_word);
                     error_flag = TRUE;
                 } else {
                     MacroLineNode *line_node = found_macro->lines_head;
                     while (line_node) {
                         fputs(line_node->line, fp_out);
                         line_node = line_node->next;
                     }
                 }
            } else {
                fputs(line, fp_out);
            }
        }
    }

    if (is_defining_macro) {
        fprintf(stderr, "Error: Macro definition started for '%s' but never ended with '%s'.\n",
                current_macro_definition ? current_macro_definition->name : "UNKNOWN", DIRECTIVE_MCROEND_STR);
        error_flag = TRUE;
    }

    fclose(fp_in);
    fclose(fp_out);
    free_macro_list(macro_list_head);

    if (error_flag) {
        fprintf(stderr, "Pre-assembly failed for '%s'. Output file '%s' may be incomplete or invalid.\n", input_filename, output_filename);
        return FALSE;
    }

    return TRUE;
}

/* --- Private Helper Function Implementations --- */
/* (Keep existing helper functions: free_macro_lines, free_macro_list, find_macro, add_macro_line, add_macro_definition) */

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
    if (!macro->lines_head) {
        macro->lines_head = new_line_node;
        macro->lines_tail = new_line_node;
    } else {
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
    new_macro->next = *head;
    *head = new_macro;
    return new_macro;
}

