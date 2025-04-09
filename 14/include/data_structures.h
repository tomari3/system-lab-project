#ifndef DATA_STRUCTURES_H
#define DATA_STRUCTURES_H

#include <stdio.h> /* Required for FILE* */
#include "constants.h" /* Include constants for enums */

/* --- Macro Structures --- */
typedef struct MacroLineNode {
    char *line;
    struct MacroLineNode *next;
} MacroLineNode;

typedef struct MacroNode {
    char *name;
    MacroLineNode *lines_head;
    MacroLineNode *lines_tail;
    struct MacroNode *next;
} MacroNode;
/* --- End Macro Structures --- */


/* --- Symbol Table Structure --- */
/** Attributes for symbols in the symbol table */
typedef enum {
    ATTR_CODE,      /* Represents an instruction address */
    ATTR_DATA,      /* Represents a data address */
    ATTR_EXTERNAL,  /* Defined in another file */
    ATTR_ENTRY,     /* Also defined as an entry point */
    ATTR_DATA_ENTRY, /* Data + Entry */
    ATTR_CODE_ENTRY  /* Code + Entry */
    /* Note: A symbol can be external OR (code/data), and can ALSO be entry */
} SymbolAttribute;

/** Node for the symbol table (linked list implementation) */
typedef struct SymbolNode {
    char *name;             /**< Label name */
    int value;              /**< Address (or 0 for external) */
    SymbolAttribute type;   /**< Base type (code, data, external) */
    int is_entry;           /**< Flag: 1 if also declared as .entry, 0 otherwise */
    struct SymbolNode *next;/**< Next node in the list */
} SymbolNode;
/* --- End Symbol Table Structure --- */


/* --- Error List Structure --- */
/** Node for storing detected errors */
typedef struct ErrorNode {
    int line_number;        /**< Line number where the error occurred */
    char *message;          /**< Error description */
    struct ErrorNode *next; /**< Next error in the list */
} ErrorNode;
/* --- End Error List Structure --- */


/* --- Parser Output Structures --- */

/** Holds information about a parsed operand */
typedef struct {
    AddressingMode mode; /**< Determined addressing mode */
    int value;           /**< Integer value (for immediate) or register number */
    char *str;           /**< String value (for label/symbol name or original immediate) */
    /* Add original text maybe? char *original_text; */
} OperandInfo;

/** Holds the structured information parsed from a single line */
typedef struct {
    int line_number;            /**< Original line number in the .am file */
    char *original_line;        /**< Copy of the original line text */
    LineType type;              /**< Type of the line (instruction, directive, etc.) */
    int has_error;              /**< Flag indicating if an error was found on this line */

    char *label;                /**< Label defined on this line (NULL if none) */

    /* Directive Info */
    DirectiveType directive_type; /**< Type of directive (if type == LINE_DIRECTIVE) */
    /* Store directive parameters appropriately - using operands for simplicity here */
    /* For .data: operands[i].value will hold the numbers */
    /* For .string: operands[0].str will hold the string content */
    /* For .entry/.extern: operands[0].str will hold the label */
    int directive_param_count;

    /* Instruction Info */
    Opcode instruction_opcode;  /**< Opcode (if type == LINE_INSTRUCTION) */
    int operand_count;          /**< Number of operands found (0, 1, or 2) */
    OperandInfo operands[MAX_OPERANDS]; /**< Parsed operand information */

} ParsedLine;

/* --- End Parser Output Structures --- */

/* --- Code/Data Image Structure --- */
typedef struct MachineWordNode {
    int address;                 /**< The memory address (IC or DC+ICF) */
    unsigned int word;           /**< The 24-bit machine code word */
    /* Add ARE bits separately if needed for output generation, */
    /* but the 'word' should contain the final encoded value including ARE */
    struct MachineWordNode *next;/**< Next word in the image */
} MachineWordNode; /* Renamed from MachineWord to avoid potential conflicts */
/* --- End Code/Data Image Structure --- */


/* --- External Symbol Usage List Structure --- */
/** Node to track usage of external symbols */
typedef struct ExternUsageNode {
    char *symbol_name;          /**< Name of the external symbol used */
    int usage_address;          /**< Address of the memory word where it's used */
    struct ExternUsageNode *next;/**< Next usage entry */
} ExternUsageNode;
/* --- End External Symbol Usage List Structure --- */


#endif /* DATA_STRUCTURES_H */
