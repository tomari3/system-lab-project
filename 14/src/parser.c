#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "../include/parser.h"
#include "../include/constants.h"
#include "../include/utils.h"
#include "../include/data_structures.h"

/* --- Private Helper Function Declarations --- */
static int is_valid_label_syntax(const char *label);
static char* extract_label(const char **line_ptr, int line_num, int *error_flag);
static Opcode get_opcode_enum(const char *opcode_str);
static DirectiveType get_directive_enum(const char *directive_str);
static OperandInfo parse_single_operand(const char *operand_str, int line_num, int *error_flag);
static void parse_instruction_operands(const char *line_ptr, ParsedLine *p_line);
static void parse_data_directive(const char *line_ptr, ParsedLine *p_line); /* Changed */
static void parse_string_directive(const char *line_ptr, ParsedLine *p_line);
static void parse_entry_extern_directive(const char *line_ptr, ParsedLine *p_line);
static void free_operand_info(OperandInfo *op_info);
static int get_register_number(const char *reg_str);

/* --- Public Functions --- */
ParsedLine* parse_line(const char *line, int line_num) {
    const char *current_pos = line;
    char first_word[MAX_LINE_LENGTH];
    int error_occurred = FALSE;
    ParsedLine *p_line = NULL;

    p_line = (ParsedLine*)safe_malloc(sizeof(ParsedLine));
    p_line->line_number = line_num;
    p_line->original_line = (char*)safe_malloc(strlen(line) + 1);
    strcpy(p_line->original_line, line);
    p_line->type = LINE_UNKNOWN;
    p_line->label = NULL;
    p_line->operand_count = 0;
    p_line->directive_param_count = 0;
    p_line->instruction_opcode = OP_UNKNOWN;
    p_line->directive_type = DIR_UNKNOWN;
    p_line->has_error = FALSE;
    memset(p_line->operands, 0, sizeof(p_line->operands));

    current_pos = skip_whitespace(current_pos);
    if (*current_pos == '\0' || *current_pos == ';') {
        p_line->type = (*current_pos == ';') ? LINE_COMMENT : LINE_EMPTY;
        return p_line;
    }

    p_line->label = extract_label(&current_pos, line_num, &error_occurred);
    if (error_occurred) {
        p_line->has_error = TRUE;
        p_line->type = LINE_ERROR;
    }
    current_pos = skip_whitespace(current_pos);

    if (*current_pos == '\0') {
        if (p_line->label) {
             fprintf(stderr, "Error (Line %d): Label '%s' defined on empty line.\n", line_num, p_line->label);
             p_line->has_error = TRUE;
        } else {
             fprintf(stderr, "Error (Line %d): Unexpected empty line content.\n", line_num);
             p_line->has_error = TRUE;
        }
         p_line->type = LINE_ERROR;
         return p_line;
    }

    if (sscanf(current_pos, "%s", first_word) != 1) {
         fprintf(stderr, "Error (Line %d): Cannot parse first command word.\n", line_num);
         p_line->has_error = TRUE;
         p_line->type = LINE_ERROR;
         return p_line;
    }
    current_pos = skip_whitespace(current_pos + strlen(first_word));

    if (first_word[0] == '.') {
        p_line->type = LINE_DIRECTIVE;
        p_line->directive_type = get_directive_enum(first_word);
        if (p_line->directive_type == DIR_UNKNOWN) {
            fprintf(stderr, "Error (Line %d): Unknown directive '%s'.\n", line_num, first_word);
            p_line->has_error = TRUE;
        } else {
            switch (p_line->directive_type) {
                case DIR_DATA:
                    parse_data_directive(current_pos, p_line); /* Call updated parser */
                    break;
                case DIR_STRING:
                    parse_string_directive(current_pos, p_line);
                    break;
                case DIR_ENTRY:
                case DIR_EXTERN:
                    if (p_line->label) {
                         fprintf(stderr, "Warning (Line %d): Label '%s' on '%s' directive line is ignored.\n", line_num, p_line->label, first_word);
                         free(p_line->label);
                         p_line->label = NULL;
                    }
                    parse_entry_extern_directive(current_pos, p_line);
                    break;
                default: p_line->has_error = TRUE; break;
            }
        }
    } else {
        p_line->type = LINE_INSTRUCTION;
        p_line->instruction_opcode = get_opcode_enum(first_word);
        if (p_line->instruction_opcode == OP_UNKNOWN) {
            fprintf(stderr, "Error (Line %d): Unknown instruction opcode '%s'.\n", line_num, first_word);
            p_line->has_error = TRUE;
        } else {
            parse_instruction_operands(current_pos, p_line);
        }
    }

    if (p_line->has_error) {
         p_line->type = LINE_ERROR;
    } else if (p_line->type == LINE_UNKNOWN) {
         fprintf(stderr, "Error (Line %d): Invalid line format or unknown command '%s'.\n", line_num, first_word);
         p_line->has_error = TRUE;
         p_line->type = LINE_ERROR;
    }
    return p_line;
}

void free_parsed_line(ParsedLine *p_line) {
    int i;
    if (!p_line) return;
    free(p_line->original_line);
    free(p_line->label);
    for (i = 0; i < p_line->operand_count; ++i) {
        free_operand_info(&p_line->operands[i]);
    }
     if(p_line->type == LINE_DIRECTIVE) {
         for (i = 0; i < p_line->directive_param_count; ++i) {
              free_operand_info(&p_line->operands[i]);
         }
     }
    free(p_line);
}

/* --- Private Helper Function Implementations --- */
static void free_operand_info(OperandInfo *op_info) {
     if (op_info && op_info->str) {
         free(op_info->str);
         op_info->str = NULL;
     }
}

static int is_valid_label_syntax(const char *label) {
    int i = 0;
    if (!label || label[0] == '\0' || strlen(label) > MAX_LABEL_LENGTH) return FALSE;
    if (!isalpha((unsigned char)label[0])) return FALSE;
    for (i = 1; label[i] != '\0'; ++i) {
        if (!isalnum((unsigned char)label[i])) return FALSE;
    }
    if (is_reserved_word(label)) return FALSE;
    return TRUE;
}

static char* extract_label(const char **line_ptr, int line_num, int *error_flag) {
    const char *start = *line_ptr;
    const char *colon_ptr = strchr(start, ':');
    char *label = NULL;
    int label_len;
    const char *p = NULL;
    if (colon_ptr) {
        label_len = colon_ptr - start;
        if (label_len > 0 && label_len <= MAX_LABEL_LENGTH) {
            p = start + label_len -1;
            while(p >= start && isspace((unsigned char)*p)) p--;
            if (p < start + label_len - 1) {
                 fprintf(stderr, "Error (Line %d): Whitespace before colon in label definition.\n", line_num);
                 *error_flag = TRUE;
                 label_len = p - start + 1;
            }
            label = (char*)safe_malloc(label_len + 1);
            strncpy(label, start, label_len);
            label[label_len] = '\0';
            if (!is_valid_label_syntax(label)) {
                fprintf(stderr, "Error (Line %d): Invalid label syntax or reserved word used for label '%s'.\n", line_num, label);
                *error_flag = TRUE;
                free(label); label = NULL;
            }
            *line_ptr = colon_ptr + 1;
        } else if (label_len > MAX_LABEL_LENGTH) {
             fprintf(stderr, "Error (Line %d): Label exceeds maximum length of %d characters.\n", line_num, MAX_LABEL_LENGTH);
             *error_flag = TRUE; *line_ptr = colon_ptr + 1;
        } else {
             fprintf(stderr, "Error (Line %d): Empty label definition (':' found at start).\n", line_num);
             *error_flag = TRUE; *line_ptr = colon_ptr + 1;
        }
    }
    return label;
}

static Opcode get_opcode_enum(const char *opcode_str) {
    if (strcmp(opcode_str, OPCODE_MOV_STR) == 0) return OP_MOV;
    if (strcmp(opcode_str, OPCODE_CMP_STR) == 0) return OP_CMP;
    if (strcmp(opcode_str, OPCODE_ADD_STR) == 0) return OP_ADD;
    if (strcmp(opcode_str, OPCODE_SUB_STR) == 0) return OP_SUB;
    if (strcmp(opcode_str, OPCODE_LEA_STR) == 0) return OP_LEA;
    if (strcmp(opcode_str, OPCODE_CLR_STR) == 0) return OP_CLR;
    if (strcmp(opcode_str, OPCODE_NOT_STR) == 0) return OP_NOT;
    if (strcmp(opcode_str, OPCODE_INC_STR) == 0) return OP_INC;
    if (strcmp(opcode_str, OPCODE_DEC_STR) == 0) return OP_DEC;
    if (strcmp(opcode_str, OPCODE_JMP_STR) == 0) return OP_JMP;
    if (strcmp(opcode_str, OPCODE_BNE_STR) == 0) return OP_BNE;
    if (strcmp(opcode_str, OPCODE_JSR_STR) == 0) return OP_JSR;
    if (strcmp(opcode_str, OPCODE_RED_STR) == 0) return OP_RED;
    if (strcmp(opcode_str, OPCODE_PRN_STR) == 0) return OP_PRN;
    if (strcmp(opcode_str, OPCODE_RTS_STR) == 0) return OP_RTS;
    if (strcmp(opcode_str, OPCODE_STP_STR) == 0) return OP_STOP;
    return OP_UNKNOWN;
}

static DirectiveType get_directive_enum(const char *directive_str) {
    if (strcmp(directive_str, DIRECTIVE_DATA_STR) == 0) return DIR_DATA;
    if (strcmp(directive_str, DIRECTIVE_STRING_STR) == 0) return DIR_STRING;
    if (strcmp(directive_str, DIRECTIVE_ENTRY_STR) == 0) return DIR_ENTRY;
    if (strcmp(directive_str, DIRECTIVE_EXTERN_STR) == 0) return DIR_EXTERN;
    return DIR_UNKNOWN;
}

static OperandInfo parse_single_operand(const char *operand_str_in, int line_num, int *error_flag) {
    OperandInfo op_info;
    char operand_str[MAX_LINE_LENGTH];
    char *str_val = NULL;
    int reg_num;
    op_info.mode = ADDR_INVALID; op_info.value = 0; op_info.str = NULL;
    if (!operand_str_in) { *error_flag = TRUE; return op_info; }
    strncpy(operand_str, operand_str_in, sizeof(operand_str) - 1);
    operand_str[sizeof(operand_str) - 1] = '\0';
    trim_whitespace(operand_str);
    if (strlen(operand_str) == 0) {
         fprintf(stderr, "Error (Line %d): Empty operand found.\n", line_num);
         *error_flag = TRUE; return op_info;
    }
    if (operand_str[0] == '#') {
        if (is_integer(operand_str + 1)) {
            op_info.mode = ADDR_IMMEDIATE; op_info.value = atoi(operand_str + 1);
        } else {
            fprintf(stderr, "Error (Line %d): Invalid immediate value '%s'.\n", line_num, operand_str + 1);
            *error_flag = TRUE;
        }
    } else if (operand_str[0] == '&') {
        str_val = operand_str + 1;
        if (is_valid_label_syntax(str_val)) {
            op_info.mode = ADDR_RELATIVE; op_info.str = (char*)safe_malloc(strlen(str_val) + 1); strcpy(op_info.str, str_val);
        } else {
            fprintf(stderr, "Error (Line %d): Invalid label '%s' for relative addressing.\n", line_num, str_val);
            *error_flag = TRUE;
        }
    } else if ((reg_num = get_register_number(operand_str)) != -1) {
        op_info.mode = ADDR_REGISTER; op_info.value = reg_num;
    } else if (is_valid_label_syntax(operand_str)) {
        op_info.mode = ADDR_DIRECT; op_info.str = (char*)safe_malloc(strlen(operand_str) + 1); strcpy(op_info.str, operand_str);
    } else {
        fprintf(stderr, "Error (Line %d): Invalid operand format '%s'.\n", line_num, operand_str);
        *error_flag = TRUE;
    }
    return op_info;
}

static int get_register_number(const char *reg_str) {
    if (strlen(reg_str) == 2 && reg_str[0] == 'r' && reg_str[1] >= '0' && reg_str[1] <= '7') {
        return reg_str[1] - '0';
    } return -1;
}

static void parse_instruction_operands(const char *line_ptr, ParsedLine *p_line) {
    int expected_count = -1;
    char line_copy[MAX_LINE_LENGTH];
    char *rest = NULL;
    int error_occurred = FALSE;
    const char *op_start = NULL;
    const char *op_end = NULL;
    const char *comma_pos = NULL;
    char op_str_buffer[MAX_LINE_LENGTH]; /* Buffer for operand string */
    int len = 0;

     switch (p_line->instruction_opcode) {
        case OP_MOV: case OP_CMP: case OP_ADD: case OP_SUB: case OP_LEA:
            expected_count = 2; break;
        case OP_CLR: case OP_NOT: case OP_INC: case OP_DEC: case OP_JMP:
        case OP_BNE: case OP_JSR: case OP_RED: case OP_PRN:
            expected_count = 1; break;
        case OP_RTS: case OP_STOP:
            expected_count = 0; break;
        default: expected_count = -1; break;
    }

    if (expected_count == -1) { p_line->has_error = TRUE; return; }

    strncpy(line_copy, line_ptr, sizeof(line_copy) - 1);
    line_copy[sizeof(line_copy) - 1] = '\0';
    rest = line_copy;
    trim_whitespace(rest);

    if (expected_count == 0) {
        if (*rest != '\0') {
            fprintf(stderr, "Error (Line %d): Extraneous text '%s' after zero-operand instruction.\n", p_line->line_number, rest);
            p_line->has_error = TRUE;
        } return;
    }

    op_start = skip_whitespace(line_ptr);
    if (*op_start == '\0' && expected_count > 0) {
         fprintf(stderr, "Error (Line %d): Missing operand(s) for instruction.\n", p_line->line_number);
         p_line->has_error = TRUE; return;
    }

     comma_pos = strchr(op_start, ',');
     if (comma_pos) { /* Potential two operands */
         op_end = comma_pos;
         len = op_end - op_start;
         if (len >= sizeof(op_str_buffer)) len = sizeof(op_str_buffer) - 1;
         strncpy(op_str_buffer, op_start, len); op_str_buffer[len] = '\0';
         p_line->operands[0] = parse_single_operand(op_str_buffer, p_line->line_number, &error_occurred);
         p_line->operand_count++; if (error_occurred) p_line->has_error = TRUE;

         op_start = skip_whitespace(comma_pos + 1);
         if (*op_start == '\0') {
              fprintf(stderr, "Error (Line %d): Missing second operand after comma.\n", p_line->line_number);
              p_line->has_error = TRUE;
         } else {
              comma_pos = strchr(op_start, ','); /* Check for extra comma */
              if (comma_pos) {
                   fprintf(stderr, "Error (Line %d): Too many commas or operands.\n", p_line->line_number);
                   p_line->has_error = TRUE;
              } else {
                   p_line->operands[1] = parse_single_operand(op_start, p_line->line_number, &error_occurred);
                   p_line->operand_count++; if (error_occurred) p_line->has_error = TRUE;
                   /* Check for trailing text after second operand */
                   op_end = op_start; while(*op_end != '\0' && !isspace((unsigned char)*op_end)) op_end++; /* Find end of operand */
                   op_end = skip_whitespace(op_end);
                   if (*op_end != '\0') {
                        fprintf(stderr, "Error (Line %d): Extraneous text after operands.\n", p_line->line_number);
                        p_line->has_error = TRUE;
                   }
              }
         }
     } else { /* Only one operand */
         p_line->operands[0] = parse_single_operand(op_start, p_line->line_number, &error_occurred);
         p_line->operand_count++; if (error_occurred) p_line->has_error = TRUE;
         /* Check for trailing text after first operand */
         op_end = op_start; while(*op_end != '\0' && !isspace((unsigned char)*op_end)) op_end++; /* Find end of operand */
         op_end = skip_whitespace(op_end);
         if (*op_end != '\0') {
             fprintf(stderr, "Error (Line %d): Extraneous text after operand.\n", p_line->line_number);
             p_line->has_error = TRUE;
         }
     }

    if (p_line->operand_count != expected_count) {
        /* Avoid duplicate error message if already flagged by missing operand checks */
        if (!p_line->has_error) {
             fprintf(stderr, "Error (Line %d): Incorrect number of operands. Expected %d, found %d.\n",
                     p_line->line_number, expected_count, p_line->operand_count);
             p_line->has_error = TRUE;
        }
    }
}

/* --- Rewritten .data parsing logic --- */
static void parse_data_directive(const char *line_ptr, ParsedLine *p_line) {
    const char *current_ptr = line_ptr;
    int count = 0;
    int state = 0; /* 0: Expecting number or end, 1: Expecting comma or end */
    long num_val = 0;
    char *end_ptr = NULL;

    while (*current_ptr != '\0') {
        current_ptr = skip_whitespace(current_ptr);
        if (*current_ptr == '\0') break; /* End of line */

        if (state == 0) { /* Expecting number */
            if (*current_ptr == ',') {
                 fprintf(stderr, "Error (Line %d): Unexpected comma in .data directive (expecting number).\n", p_line->line_number);
                 p_line->has_error = TRUE; return;
            }
            /* Try parsing number */
            num_val = strtol(current_ptr, &end_ptr, 10);
            if (end_ptr == current_ptr) { /* No number parsed */
                fprintf(stderr, "Error (Line %d): Expected number but found '%c' in .data directive.\n", p_line->line_number, *current_ptr);
                p_line->has_error = TRUE; return;
            }
            /* Check range? (Assuming int is sufficient for 24-bit signed) */

            /* Store number */
            if (count < MAX_MEMORY_WORDS) {
                 p_line->operands[count].mode = ADDR_IMMEDIATE;
                 p_line->operands[count].value = (int)num_val;
                 p_line->operands[count].str = NULL;
                 count++;
            } else {
                 fprintf(stderr, "Error (Line %d): Too many data values.\n", p_line->line_number);
                 p_line->has_error = TRUE; return; /* Stop parsing this line */
            }
            current_ptr = end_ptr; /* Advance pointer past number */
            state = 1; /* Now expect comma or end */

        } else { /* state == 1: Expecting comma or end */
            if (*current_ptr != ',') {
                fprintf(stderr, "Error (Line %d): Expected comma or end of line but found '%c' in .data directive.\n", p_line->line_number, *current_ptr);
                p_line->has_error = TRUE; return;
            }
            /* Found comma */
            current_ptr++;
            state = 0; /* Now expect number */
        }
    } /* End while */

    /* Final state check */
    if (state == 0 && count > 0) { /* Ended expecting number after a comma */
         fprintf(stderr, "Error (Line %d): Trailing comma found in .data directive.\n", p_line->line_number);
         p_line->has_error = TRUE;
    }
    if (count == 0 && !p_line->has_error) { /* Parsed nothing successfully */
         /* Check if the line wasn't just whitespace */
         current_ptr = skip_whitespace(line_ptr);
         if (*current_ptr != '\0') {
              fprintf(stderr, "Error (Line %d): No valid numbers found for .data directive.\n", p_line->line_number);
              p_line->has_error = TRUE;
         } else {
              /* Line only contained whitespace after directive - technically valid but maybe warn? */
              fprintf(stderr, "Warning (Line %d): No data values provided for .data directive.\n", p_line->line_number);
         }
    }

    p_line->directive_param_count = count;
}
/* --- End rewritten .data parsing logic --- */


static void parse_string_directive(const char *line_ptr, ParsedLine *p_line) {
    const char *start = skip_whitespace(line_ptr);
    const char *end = NULL;
    char *str_content = NULL;
    int len = 0;
    const char *after_quote = NULL;
    if (*start != '"') {
        fprintf(stderr, "Error (Line %d): String directive parameter must start with '\"'. Found '%c'.\n", p_line->line_number, *start);
        p_line->has_error = TRUE; return;
    }
    end = strchr(start + 1, '"');
    if (!end) {
        fprintf(stderr, "Error (Line %d): String directive parameter must end with '\"'.\n", p_line->line_number);
        p_line->has_error = TRUE; return;
    }
    after_quote = skip_whitespace(end + 1);
    if (*after_quote != '\0') {
         fprintf(stderr, "Error (Line %d): Extraneous text found after closing quote of string directive.\n", p_line->line_number);
         p_line->has_error = TRUE;
    }
    len = end - (start + 1); if (len < 0) len = 0;
    str_content = (char*)safe_malloc(len + 1);
    strncpy(str_content, start + 1, len); str_content[len] = '\0';
    p_line->operands[0].mode = ADDR_NONE; p_line->operands[0].str = str_content;
    p_line->operands[0].value = 0; p_line->directive_param_count = 1;
}

static void parse_entry_extern_directive(const char *line_ptr, ParsedLine *p_line) {
    char label_name[MAX_LABEL_LENGTH + 1];
    const char *start = skip_whitespace(line_ptr);
    const char *end = NULL;
    if (sscanf(start, "%s", label_name) != 1) {
         fprintf(stderr, "Error (Line %d): Missing label parameter for %s directive.\n", p_line->line_number,
                 (p_line->directive_type == DIR_ENTRY) ? DIRECTIVE_ENTRY_STR : DIRECTIVE_EXTERN_STR);
         p_line->has_error = TRUE; return;
    }
    end = skip_whitespace(start + strlen(label_name));
     if (*end != '\0') {
         fprintf(stderr, "Error (Line %d): Extraneous text after label '%s' in %s directive.\n", p_line->line_number, label_name,
                 (p_line->directive_type == DIR_ENTRY) ? DIRECTIVE_ENTRY_STR : DIRECTIVE_EXTERN_STR);
         p_line->has_error = TRUE;
     }
     if (!is_valid_label_syntax(label_name)) {
          if (!is_reserved_word(label_name)) {
               fprintf(stderr, "Error (Line %d): Invalid label syntax for '%s' parameter '%s'.\n", p_line->line_number,
                       (p_line->directive_type == DIR_ENTRY) ? DIRECTIVE_ENTRY_STR : DIRECTIVE_EXTERN_STR, label_name);
               p_line->has_error = TRUE;
          }
     }
    p_line->operands[0].mode = ADDR_DIRECT;
    p_line->operands[0].str = (char*)safe_malloc(strlen(label_name) + 1);
    strcpy(p_line->operands[0].str, label_name);
    p_line->operands[0].value = 0; p_line->directive_param_count = 1;
}

