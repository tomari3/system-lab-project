#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/second_pass.h"
#include "../include/parser.h"
#include "../include/symbol_table.h"
#include "../include/encoding.h"
#include "../include/constants.h"
#include "../include/utils.h"

/* --- Private Helper Function Declarations --- */
static MachineWordNode* append_machine_word(MachineWordNode **head, MachineWordNode **tail, int address, unsigned int word);
static ExternUsageNode* add_extern_usage(ExternUsageNode **head, const char *symbol_name, int usage_address);
static int get_instruction_length(ParsedLine *p_line);

/* --- Public Function Implementation --- */
int run_second_pass(const char *am_filename, SymbolNode *symbol_table,
                    MachineWordNode **code_image_head, MachineWordNode **data_image_head,
                    ExternUsageNode **extern_list_head, int icf, int dcf)
{
    FILE *fp_am = NULL;
    char line[MAX_LINE_LENGTH];
    int line_num = 0;
    int ic = MEMORY_START_ADDRESS;
    int dc = 0;
    int error_flag = FALSE;
    ParsedLine *p_line = NULL;
    MachineWordNode *code_tail = NULL;
    MachineWordNode *data_tail = NULL;

    *code_image_head = NULL;
    *data_image_head = NULL;
    *extern_list_head = NULL;

    fp_am = fopen(am_filename, "r");
    if (!fp_am) {
        fprintf(stderr, "Error: Could not open intermediate file '%s' for reading (Second Pass).\n", am_filename);
        return FALSE;
    }

    printf("Starting Second Pass...\n");

    while (fgets(line, sizeof(line), fp_am)) {
        line_num++;
        p_line = parse_line(line, line_num);

        if (!p_line) {
             fprintf(stderr, "Fatal Error (Line %d): Memory allocation failed during parsing (Second Pass).\n", line_num);
             error_flag = TRUE; break;
        }
        if (p_line->has_error || p_line->type == LINE_ERROR) {
            error_flag = TRUE; free_parsed_line(p_line); continue;
        }

        switch (p_line->type) {
            case LINE_INSTRUCTION: {
                /* --- Moved Declarations --- */
                int current_ic = ic;
                int instruction_len = get_instruction_length(p_line);
                unsigned int first_word = 0;
                unsigned int operand_word = 0;
                SymbolNode *symbol = NULL;
                int value_to_encode = 0;
                OperandInfo *op1 = (p_line->operand_count > 0) ? &p_line->operands[0] : NULL;
                OperandInfo *op2 = (p_line->operand_count > 1) ? &p_line->operands[1] : NULL;
                int src_reg = (op1 && op1->mode == ADDR_REGISTER) ? op1->value : 0;
                int dest_reg = 0;
                AddressingMode src_mode = (op1) ? op1->mode : ADDR_NONE;
                AddressingMode dest_mode = ADDR_NONE;
                int is_reg_reg = FALSE; /* Initialize */
                OperandInfo* dest_op = NULL; /* Initialize */
                /* --- End Moved Declarations --- */

                if (p_line->operand_count == 1) {
                    dest_mode = op1->mode;
                    dest_reg = (op1->mode == ADDR_REGISTER) ? op1->value : 0;
                    src_mode = ADDR_NONE; src_reg = 0;
                } else if (p_line->operand_count == 2) {
                    dest_mode = op2->mode;
                    dest_reg = (op2->mode == ADDR_REGISTER) ? op2->value : 0;
                }

                first_word = encode_instruction_first_word(p_line->instruction_opcode, src_mode, src_reg, dest_mode, dest_reg);
                if (!append_machine_word(code_image_head, &code_tail, current_ic, first_word)) {
                    error_flag = TRUE; break;
                }
                ic++;

                is_reg_reg = (p_line->operand_count == 2 && src_mode == ADDR_REGISTER && dest_mode == ADDR_REGISTER);

                /* Encode source operand word */
                if (op1 && !is_reg_reg && (src_mode == ADDR_IMMEDIATE || src_mode == ADDR_DIRECT || src_mode == ADDR_RELATIVE)) {
                    symbol = NULL; value_to_encode = 0;
                    if (src_mode == ADDR_IMMEDIATE) {
                        value_to_encode = op1->value;
                    } else {
                        symbol = find_symbol(symbol_table, op1->str);
                        if (!symbol) {
                             fprintf(stderr, "Error (Line %d): Undefined symbol '%s' used as operand.\n", line_num, op1->str);
                             error_flag = TRUE;
                        } else {
                            if (src_mode == ADDR_RELATIVE) {
                                if (symbol->type == ATTR_EXTERNAL) {
                                     fprintf(stderr, "Error (Line %d): Cannot use relative addressing with external symbol '%s'.\n", line_num, op1->str);
                                     error_flag = TRUE;
                                } else { value_to_encode = symbol->value - current_ic; }
                            } else { /* Direct */
                                 value_to_encode = symbol->value;
                                 if (symbol->type == ATTR_EXTERNAL) {
                                      if (!add_extern_usage(extern_list_head, symbol->name, ic)) error_flag = TRUE;
                                 }
                            }
                        }
                    }
                    if (!error_flag) {
                         operand_word = encode_operand_word(value_to_encode, symbol, src_mode);
                         if (!append_machine_word(code_image_head, &code_tail, ic, operand_word)) error_flag = TRUE;
                         ic++;
                    }
                }

                /* Encode destination operand word */
                dest_op = (p_line->operand_count == 1) ? op1 : op2;
                if (dest_op && !is_reg_reg && (dest_mode == ADDR_IMMEDIATE || dest_mode == ADDR_DIRECT || dest_mode == ADDR_RELATIVE)) {
                     symbol = NULL; value_to_encode = 0;
                     if (dest_mode == ADDR_IMMEDIATE) {
                         value_to_encode = dest_op->value;
                     } else {
                         symbol = find_symbol(symbol_table, dest_op->str);
                         if (!symbol) {
                              fprintf(stderr, "Error (Line %d): Undefined symbol '%s' used as operand.\n", line_num, dest_op->str);
                              error_flag = TRUE;
                         } else {
                             if (dest_mode == ADDR_RELATIVE) {
                                 if (symbol->type == ATTR_EXTERNAL) {
                                      fprintf(stderr, "Error (Line %d): Cannot use relative addressing with external symbol '%s'.\n", line_num, dest_op->str);
                                      error_flag = TRUE;
                                 } else { value_to_encode = symbol->value - current_ic; }
                             } else { /* Direct */
                                  value_to_encode = symbol->value;
                                  if (symbol->type == ATTR_EXTERNAL) {
                                       if (!add_extern_usage(extern_list_head, symbol->name, ic)) error_flag = TRUE;
                                  }
                             }
                         }
                     }
                     if (!error_flag) {
                          operand_word = encode_operand_word(value_to_encode, symbol, dest_mode);
                          if (!append_machine_word(code_image_head, &code_tail, ic, operand_word)) error_flag = TRUE;
                          ic++;
                     }
                }

                if (ic != current_ic + instruction_len) {
                     fprintf(stderr, "Internal Error (Line %d): IC mismatch. Expected %d, got %d.\n", line_num, current_ic + instruction_len, ic);
                     error_flag = TRUE; ic = current_ic + instruction_len;
                }
                break;
            }

            case LINE_DIRECTIVE: {
                int i; /* C90 requires declaration at top of block */
                if (p_line->directive_type == DIR_DATA) {
                    for (i = 0; i < p_line->directive_param_count; ++i) {
                        unsigned int data_word = encode_data_word(p_line->operands[i].value);
                        if (!append_machine_word(data_image_head, &data_tail, icf + dc, data_word)) {
                            error_flag = TRUE; break;
                        } dc++;
                    }
                } else if (p_line->directive_type == DIR_STRING) {
                    const char *str = p_line->operands[0].str;
                    if (str) {
                        for (i = 0; str[i] != '\0'; ++i) {
                             unsigned int data_word = encode_data_word((int)str[i]);
                             if (!append_machine_word(data_image_head, &data_tail, icf + dc, data_word)) {
                                 error_flag = TRUE; break;
                             } dc++;
                        }
                        if (!error_flag) {
                             unsigned int null_word = encode_data_word(0);
                             if (!append_machine_word(data_image_head, &data_tail, icf + dc, null_word)) {
                                 error_flag = TRUE;
                             } dc++;
                        }
                    }
                } else if (p_line->directive_type == DIR_ENTRY) {
                    if (p_line->directive_param_count == 1 && p_line->operands[0].str) {
                        mark_symbol_as_entry(symbol_table, p_line->operands[0].str, line_num, &error_flag);
                    }
                }
                break;
            }

            case LINE_EMPTY: case LINE_COMMENT: break;
            default: error_flag = TRUE; break;
        }
        free_parsed_line(p_line); p_line = NULL;
        if (error_flag) break;
    }

    fclose(fp_am);

    if (error_flag) {
        printf("Second Pass failed with errors.\n");
        free_machine_word_list(*code_image_head); *code_image_head = NULL;
        free_machine_word_list(*data_image_head); *data_image_head = NULL;
        free_extern_list(*extern_list_head); *extern_list_head = NULL;
        return FALSE;
    }

    printf("Second Pass successful.\n");
    return TRUE;
}

/* --- List Management Helper Functions --- */
static MachineWordNode* append_machine_word(MachineWordNode **head, MachineWordNode **tail, int address, unsigned int word) {
    MachineWordNode *new_node = (MachineWordNode*)safe_malloc(sizeof(MachineWordNode));
    new_node->address = address; new_node->word = word; new_node->next = NULL;
    if (*head == NULL) { *head = new_node; *tail = new_node; }
    else { (*tail)->next = new_node; *tail = new_node; }
    return new_node;
}

static ExternUsageNode* add_extern_usage(ExternUsageNode **head, const char *symbol_name, int usage_address) {
    ExternUsageNode *new_node = (ExternUsageNode*)safe_malloc(sizeof(ExternUsageNode));
    new_node->symbol_name = (char*)safe_malloc(strlen(symbol_name) + 1);
    strcpy(new_node->symbol_name, symbol_name);
    new_node->usage_address = usage_address; new_node->next = *head; *head = new_node;
    return new_node;
}

void free_machine_word_list(MachineWordNode *head) {
    MachineWordNode *current = head, *next_node = NULL;
    while (current != NULL) { next_node = current->next; free(current); current = next_node; }
}

void free_extern_list(ExternUsageNode *head) {
    ExternUsageNode *current = head, *next_node = NULL;
    while (current != NULL) { next_node = current->next; free(current->symbol_name); free(current); current = next_node; }
}

/* --- Other Helper Functions --- */
static int get_instruction_length(ParsedLine *p_line) {
    int length = 1;
    OperandInfo *op1 = (p_line->operand_count > 0) ? &p_line->operands[0] : NULL;
    OperandInfo *op2 = (p_line->operand_count > 1) ? &p_line->operands[1] : NULL;
    if (p_line->operand_count == 2 && op1->mode == ADDR_REGISTER && op2->mode == ADDR_REGISTER) return 1;
    if (op1 && (op1->mode == ADDR_IMMEDIATE || op1->mode == ADDR_DIRECT || op1->mode == ADDR_RELATIVE)) length++;
    if (op2 && (op2->mode == ADDR_IMMEDIATE || op2->mode == ADDR_DIRECT || op2->mode == ADDR_RELATIVE)) length++;
    return length;
}

