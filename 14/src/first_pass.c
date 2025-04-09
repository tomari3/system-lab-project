#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/first_pass.h"
#include "../include/parser.h"
#include "../include/symbol_table.h"
#include "../include/constants.h"
#include "../include/utils.h"

/* --- Private Helper Function Declarations --- */
static int calculate_instruction_length(ParsedLine *p_line);
static int validate_addressing_modes(ParsedLine *p_line);
static int is_mode_valid_for_operand(Opcode op, int operand_index, AddressingMode mode);
static int get_expected_operand_count(Opcode op);
/* --- Added prototype for getting opcode string --- */
static const char* get_opcode_string(Opcode op);


/* --- Public Function Implementation --- */

int run_first_pass(const char *am_filename, SymbolNode **symbol_table_head, int *icf, int *dcf) {
    FILE *fp_am = NULL;
    char line[MAX_LINE_LENGTH];
    int line_num = 0;
    int ic = MEMORY_START_ADDRESS;
    int dc = 0;
    int error_flag = FALSE;
    ParsedLine *p_line = NULL;
    SymbolNode *new_sym = NULL;

    *symbol_table_head = NULL;
    *icf = 0;
    *dcf = 0;

    fp_am = fopen(am_filename, "r");
    if (!fp_am) {
        fprintf(stderr, "Error: Could not open intermediate file '%s' for reading.\n", am_filename);
        return FALSE;
    }

    printf("Starting First Pass...\n");

    while (fgets(line, sizeof(line), fp_am)) {
        line_num++;
        p_line = parse_line(line, line_num);

        if (!p_line) {
             fprintf(stderr, "Fatal Error (Line %d): Memory allocation failed during parsing.\n", line_num);
             error_flag = TRUE;
             break;
        }

        if (p_line->has_error) {
            error_flag = TRUE;
        }

        if (p_line->label != NULL) {
            if (p_line->type == LINE_INSTRUCTION) {
                new_sym = add_symbol(symbol_table_head, p_line->label, ic, ATTR_CODE, line_num, &error_flag);
                if (!new_sym && !error_flag) error_flag = TRUE;
            } else if (p_line->type == LINE_DIRECTIVE &&
                       (p_line->directive_type == DIR_DATA || p_line->directive_type == DIR_STRING)) {
                new_sym = add_symbol(symbol_table_head, p_line->label, dc, ATTR_DATA, line_num, &error_flag);
                 if (!new_sym && !error_flag) error_flag = TRUE;
            }
        }

        switch (p_line->type) {
            case LINE_INSTRUCTION:
                if (!p_line->has_error) {
                    /* Validate addressing modes *before* calculating length */
                    if (!validate_addressing_modes(p_line)) {
                         error_flag = TRUE; /* Validation function sets p_line->has_error */
                    } else {
                        /* Only calculate length if modes are valid */
                        int instruction_len = calculate_instruction_length(p_line);
                        ic += instruction_len;
                    }
                }
                break;

            case LINE_DIRECTIVE:
                if (!p_line->has_error) {
                    if (p_line->directive_type == DIR_DATA) {
                        dc += p_line->directive_param_count;
                    } else if (p_line->directive_type == DIR_STRING) {
                        if (p_line->operands[0].str != NULL) {
                             dc += strlen(p_line->operands[0].str) + 1;
                        }
                    } else if (p_line->directive_type == DIR_EXTERN) {
                         if (p_line->directive_param_count == 1 && p_line->operands[0].str) {
                              new_sym = add_symbol(symbol_table_head, p_line->operands[0].str, 0, ATTR_EXTERNAL, line_num, &error_flag);
                               if (!new_sym && !error_flag) error_flag = TRUE;
                         }
                    }
                }
                break;

            case LINE_EMPTY:
            case LINE_COMMENT:
                break;

            case LINE_ERROR: /* Error already flagged by parser */
                 error_flag = TRUE;
                 break;
            case LINE_UNKNOWN:
                 error_flag = TRUE;
                 break;
        }

        free_parsed_line(p_line);
        p_line = NULL;

    }

    fclose(fp_am);

    if (error_flag) {
        printf("First Pass failed with errors.\n");
        free_symbol_table(*symbol_table_head);
        *symbol_table_head = NULL;
        return FALSE;
    }

    *icf = ic;
    *dcf = dc;
    relocate_data_symbols(*symbol_table_head, *icf);

    printf("First Pass successful. ICF = %d, DCF = %d\n", *icf, *dcf);
    return TRUE;
}


/* --- Private Helper Function Implementations --- */

static int get_expected_operand_count(Opcode op) {
    switch (op) {
        case OP_MOV: case OP_CMP: case OP_ADD: case OP_SUB: case OP_LEA:
            return 2;
        case OP_CLR: case OP_NOT: case OP_INC: case OP_DEC: case OP_JMP:
        case OP_BNE: case OP_JSR: case OP_RED: case OP_PRN:
            return 1;
        case OP_RTS: case OP_STOP:
            return 0;
        default:
            return -1;
    }
}

static int calculate_instruction_length(ParsedLine *p_line) {
    int length = 1;
    OperandInfo *op1 = (p_line->operand_count > 0) ? &p_line->operands[0] : NULL;
    OperandInfo *op2 = (p_line->operand_count > 1) ? &p_line->operands[1] : NULL;

    /* Check if both are registers - special case, only 1 word total */
    /* NOTE: This rule wasn't explicitly in the PDF but is common in assemblers */
    /* and simplifies encoding. If this causes issues, remove this check. */
    if (p_line->operand_count == 2 && op1->mode == ADDR_REGISTER && op2->mode == ADDR_REGISTER) {
        return 1;
    }

    /* Check first operand */
    if (op1 && (op1->mode == ADDR_IMMEDIATE || op1->mode == ADDR_DIRECT || op1->mode == ADDR_RELATIVE)) {
        length++;
    }

    /* Check second operand */
    if (op2 && (op2->mode == ADDR_IMMEDIATE || op2->mode == ADDR_DIRECT || op2->mode == ADDR_RELATIVE)) {
        length++;
    }

    return length;
}

/** Validates addressing modes for a given instruction line. */
static int validate_addressing_modes(ParsedLine *p_line) {
     int is_valid = TRUE;
     Opcode op = p_line->instruction_opcode;
     int expected_count = get_expected_operand_count(op);
     const char *op_str = get_opcode_string(op); /* Get opcode name for errors */

     /* Check if actual operand count matches expected (parser should catch this, but double check) */
     if (p_line->operand_count != expected_count) {
         /* This case should ideally be handled by the parser */
         fprintf(stderr, "Internal Error (Line %d): Operand count mismatch for %s (expected %d, parsed %d).\n",
                 p_line->line_number, op_str, expected_count, p_line->operand_count);
         p_line->has_error = TRUE;
         return FALSE;
     }

     /* Validate based on expected count */
     if (expected_count == 2) {
         /* Validate source operand (index 0) */
         if (!is_mode_valid_for_operand(op, 0, p_line->operands[0].mode)) {
              fprintf(stderr, "Error (Line %d): Invalid addressing mode (%d) for source operand of %s.\n",
                      p_line->line_number, p_line->operands[0].mode, op_str);
              is_valid = FALSE;
              p_line->has_error = TRUE;
         }
         /* Validate destination operand (index 1) */
         if (!is_mode_valid_for_operand(op, 1, p_line->operands[1].mode)) {
              fprintf(stderr, "Error (Line %d): Invalid addressing mode (%d) for destination operand of %s.\n",
                      p_line->line_number, p_line->operands[1].mode, op_str);
              is_valid = FALSE;
              p_line->has_error = TRUE;
         }
     } else if (expected_count == 1) {
          /* Validate the single operand (treat as destination, index 1) */
          if (!is_mode_valid_for_operand(op, 1, p_line->operands[0].mode)) {
               fprintf(stderr, "Error (Line %d): Invalid addressing mode (%d) for operand of %s.\n",
                      p_line->line_number, p_line->operands[0].mode, op_str);
               is_valid = FALSE;
               p_line->has_error = TRUE;
          }
     }
     /* No validation needed for expected_count == 0 */

     return is_valid;
}

/** Checks if a specific addressing mode is valid for a given operand type/index. */
static int is_mode_valid_for_operand(Opcode op, int operand_index, AddressingMode mode) {
    /* operand_index: 0 for source, 1 for destination */
    switch (op) {
        /* Group 1: mov, add, sub (Source: 0,1,3 | Dest: 1,3) */
        case OP_MOV:
        case OP_ADD:
        case OP_SUB:
            if (operand_index == 0) return (mode == ADDR_IMMEDIATE || mode == ADDR_DIRECT || mode == ADDR_REGISTER);
            else /* operand_index == 1 */ return (mode == ADDR_DIRECT || mode == ADDR_REGISTER);

        /* Group 2: cmp (Source: 0,1,3 | Dest: 0,1,3) */
        case OP_CMP:
             return (mode == ADDR_IMMEDIATE || mode == ADDR_DIRECT || mode == ADDR_REGISTER);

        /* Group 3: lea (Source: 1 | Dest: 1,3) */
        case OP_LEA:
             if (operand_index == 0) return (mode == ADDR_DIRECT);
             else /* operand_index == 1 */ return (mode == ADDR_DIRECT || mode == ADDR_REGISTER);

        /* Group 4: Single Operand (Dest: 1,3) - clr, not, inc, dec, red */
        case OP_CLR:
        case OP_NOT:
        case OP_INC:
        case OP_DEC:
        case OP_RED:
             if (operand_index == 1) return (mode == ADDR_DIRECT || mode == ADDR_REGISTER);
             else return FALSE; /* No source operand */

        /* Group 5: Single Operand (Dest: 1,2) - jmp, bne, jsr */
        case OP_JMP:
        case OP_BNE:
        case OP_JSR:
             if (operand_index == 1) return (mode == ADDR_DIRECT || mode == ADDR_RELATIVE);
             else return FALSE; /* No source operand */

        /* Group 6: Single Operand (Dest: 0,1,3) - prn */
        case OP_PRN:
             if (operand_index == 1) return (mode == ADDR_IMMEDIATE || mode == ADDR_DIRECT || mode == ADDR_REGISTER);
             else return FALSE; /* No source operand */

        /* Group 7: Zero Operand - rts, stop */
        case OP_RTS:
        case OP_STOP:
             return FALSE; /* No operands allowed */

        /* Default: Unknown Opcode */
        default:
            return FALSE;
    }
}

/* --- Added helper to get opcode string for error messages --- */
static const char* get_opcode_string(Opcode op) {
     switch (op) {
        case OP_MOV: return OPCODE_MOV_STR;
        case OP_CMP: return OPCODE_CMP_STR;
        case OP_ADD: return OPCODE_ADD_STR;
        case OP_SUB: return OPCODE_SUB_STR;
        case OP_LEA: return OPCODE_LEA_STR;
        case OP_CLR: return OPCODE_CLR_STR;
        case OP_NOT: return OPCODE_NOT_STR;
        case OP_INC: return OPCODE_INC_STR;
        case OP_DEC: return OPCODE_DEC_STR;
        case OP_JMP: return OPCODE_JMP_STR;
        case OP_BNE: return OPCODE_BNE_STR;
        case OP_JSR: return OPCODE_JSR_STR;
        case OP_RED: return OPCODE_RED_STR;
        case OP_PRN: return OPCODE_PRN_STR;
        case OP_RTS: return OPCODE_RTS_STR;
        case OP_STOP: return OPCODE_STP_STR;
        default: return "UNKNOWN";
     }
}


