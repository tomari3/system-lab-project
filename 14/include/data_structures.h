#ifndef DATA_STRUCTURES_H
#define DATA_STRUCTURES_H

#include <stdio.h> 

/**
 * @brief Represents a single line within a macro definition.
 */
typedef struct MacroLineNode {
    char *line;                  /**< The content of the line */
    struct MacroLineNode *next; /**< Pointer to the next line in the macro */
} MacroLineNode;

/**
 * @brief Represents a macro definition.
 */
typedef struct MacroNode {
    char *name;                 /**< The name of the macro */
    MacroLineNode *lines_head;  /**< Pointer to the first line of the macro body */
    MacroLineNode *lines_tail;  /**< Pointer to the last line (for easy appending) */
    struct MacroNode *next;     /**< Pointer to the next macro definition in the list */
} MacroNode;

/* --- End Added Macro Structures --- */


/* --- Placeholder for Symbol Table Structure --- */
/* TODO: Define Symbol Table structure and related types */
typedef struct SymbolNode {
    char *name;
    int value;
    /* Add attributes like code, data, entry, external */
    struct SymbolNode *next;
} SymbolNode;
/* --- End Placeholder --- */


/* --- Placeholder for Error List Structure --- */
/* TODO: Define Error List structure */
typedef struct ErrorNode {
    int line_number;
    char *message;
    struct ErrorNode *next;
} ErrorNode;
/* --- End Placeholder --- */


/* --- Placeholder for Instruction/Data Image Structure --- */
/* TODO: Define structures to hold the encoded machine words */
/* Example:
typedef struct MachineWord {
    int address;
    unsigned int bits : 24; // Using bit-field for 24 bits
    // Or use unsigned int and mask later
    // Add ARE info if needed separately
} MachineWord;
*/
/* --- End Placeholder --- */


#endif /* DATA_STRUCTURES_H */


