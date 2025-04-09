#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/symbol_table.h"
#include "../include/utils.h" /* For safe_malloc */

/**
 * @brief Adds a symbol to the symbol table (linked list).
 * Checks for redefinition errors. Allocates memory for the new node and name.
 */
SymbolNode* add_symbol(SymbolNode **head, const char *name, int value, SymbolAttribute type, int line_num, int *error_flag) {
    SymbolNode *new_node = NULL;
    SymbolNode *existing = NULL;

    /* Check if symbol already exists */
    existing = find_symbol(*head, name);
    if (existing) {
        /* Allow defining an external symbol multiple times (usually harmless) */
        /* But error if defined again as code/data or vice-versa */
        if (existing->type != ATTR_EXTERNAL || type != ATTR_EXTERNAL) {
             fprintf(stderr, "Error (Line %d): Symbol '%s' redefined.\n", line_num, name);
             *error_flag = TRUE;
             return NULL;
        }
         /* If external is defined again as external, just ignore */
         return existing;
    }

    /* Allocate new node */
    new_node = (SymbolNode*)safe_malloc(sizeof(SymbolNode));
    new_node->name = (char*)safe_malloc(strlen(name) + 1);
    strcpy(new_node->name, name);
    new_node->value = value;
    new_node->type = type;
    new_node->is_entry = FALSE; /* Initially not an entry point */
    new_node->next = *head; /* Add to beginning of list */
    *head = new_node;

    return new_node;
}

/**
 * @brief Finds a symbol by name in the symbol table.
 */
SymbolNode* find_symbol(SymbolNode *head, const char *name) {
    SymbolNode *current = head;
    while (current != NULL) {
        if (strcmp(current->name, name) == 0) {
            return current;
        }
        current = current->next;
    }
    return NULL; /* Not found */
}

/**
 * @brief Marks an existing symbol as also being an entry point.
 */
void mark_symbol_as_entry(SymbolNode *head, const char *name, int line_num, int *error_flag) {
     SymbolNode *symbol = find_symbol(head, name);

     if (!symbol) {
         fprintf(stderr, "Error (Line %d): '.entry' directive for undefined symbol '%s'.\n", line_num, name);
         *error_flag = TRUE;
         return;
     }

     if (symbol->type == ATTR_EXTERNAL) {
          fprintf(stderr, "Error (Line %d): Symbol '%s' declared as '.entry' cannot also be external.\n", line_num, name);
          *error_flag = TRUE;
          return;
     }

     /* Mark as entry */
     symbol->is_entry = TRUE;

     /* Optional: Update type for clarity if needed, though is_entry flag is primary */
     /* if (symbol->type == ATTR_CODE) symbol->type = ATTR_CODE_ENTRY; */
     /* else if (symbol->type == ATTR_DATA) symbol->type = ATTR_DATA_ENTRY; */
}


/**
 * @brief Relocates (updates) the addresses of all data symbols by adding the
 * final instruction counter value (ICF).
 */
void relocate_data_symbols(SymbolNode *head, int icf_value) {
     SymbolNode *current = head;
     while (current != NULL) {
         /* Only relocate data symbols, not code or external */
         if (current->type == ATTR_DATA) {
             current->value += icf_value;
         }
         current = current->next;
     }
}


/**
 * @brief Frees all memory associated with the symbol table linked list.
 */
void free_symbol_table(SymbolNode *head) {
    SymbolNode *current = head;
    SymbolNode *next_node = NULL;

    while (current != NULL) {
        next_node = current->next;
        free(current->name); /* Free the duplicated name string */
        free(current);       /* Free the node itself */
        current = next_node;
    }
}

