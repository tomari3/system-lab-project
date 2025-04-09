#include "../include/preprocessor.h"
#include "../include/assembler.h"
#include "../include/utils.h"
#include <string.h>
#include <ctype.h>
#include <stdio.h>

/* Add a macro definition to the macro table */
bool addMacro(AssemblerContext* context, const char* name, const char* content) {
    MacroEntry* newEntry;
    char errorMsg[300];
    
    if (!context || !name || !*name || !content) {
        return false;
    }
    
    /* Check if macro already exists */
    if (findMacro(context, name) != NULL) {
        sprintf(errorMsg, "Macro '%s' already defined", name);
        addError(context, errorMsg);
        return false;
    }
    
    /* Check if the name is a reserved word */
    if (isOperation(name) || isDirective(name) || isRegister(name)) {
        sprintf(errorMsg, "Macro name '%s' is a reserved word", name);
        addError(context, errorMsg);
        return false;
    }
    
    /* Create new macro entry */
    newEntry = (MacroEntry*)malloc(sizeof(MacroEntry));
    if (!newEntry) {
        addError(context, "Memory allocation failed for macro table");
        return false;
    }
    
    /* Initialize the new entry */
    strncpy(newEntry->name, name, MAX_LABEL_LENGTH - 1);
    newEntry->name[MAX_LABEL_LENGTH - 1] = '\0';
    
    newEntry->content = my_strdup(content);
    if (!newEntry->content) {
        free(newEntry);
        addError(context, "Memory allocation failed for macro content");
        return false;
    }
    
    /* Add to the beginning of the list */
    newEntry->next = context->macroTable;
    context->macroTable = newEntry;
    
    return true;
}

/* Find a macro in the macro table by name */
MacroEntry* findMacro(AssemblerContext* context, const char* name) {
    MacroEntry* current;
    
    if (!context || !name || !*name) {
        return NULL;
    }
    
    current = context->macroTable;
    while (current) {
        if (strcmp(current->name, name) == 0) {
            return current;
        }
        current = current->next;
    }
    
    return NULL;
}

/* Free the macro table */
void freeMacroTable(MacroEntry* head) {
    MacroEntry* current = head;
    MacroEntry* next;
    
    while (current) {
        next = current->next;
        free(current->content);
        free(current);
        current = next;
    }
}

/* Remove leading and trailing whitespace */
static void trim(char* str) {
    char* start = str;
    char* end;
    
    if (!str || !*str) return;
    
    /* Skip leading whitespace */
    while(isspace((unsigned char)*start)) start++;
    
    /* All whitespace? */
    if(!*start) {
        *str = '\0';
        return;
    }
    
    /* Move to end */
    end = start + strlen(start) - 1;
    /* Remove trailing whitespace */
    while(end > start && isspace((unsigned char)*end)) end--;
    
    /* Terminate string after end */
    *(end + 1) = '\0';
    
    /* Move string to beginning if needed */
    if(start > str) {
        memmove(str, start, (end - start) + 2);
    }
}

/* Preprocessor main function - handles macro expansion */
bool preprocess(AssemblerContext* context) {
    FILE* sourceFile;
    FILE* amFile;
    char line[MAX_LINE_LENGTH];
    char cleanLine[MAX_LINE_LENGTH];
    char macroName[MAX_LABEL_LENGTH];
    char macroContent[10000] = ""; /* Buffer for macro content */
    bool inMacroDefinition = false;
    int lineNumber = 0;
    char* token;
    char* saveptr;
    MacroEntry* macro;
    char* firstToken;
    
    if (!context) {
        return false;
    }
    
    sourceFile = context->sourceFile;
    
    /* Create the .am file for expanded source */
    amFile = context->amFile;
    if (!amFile) {
        addError(context, "Failed to create .am file");
        return false;
    }
    
    /* Reset file positions */
    rewind(sourceFile);
    
    /* Process the source file line by line */
    while (fgets(line, MAX_LINE_LENGTH, sourceFile)) {
        lineNumber++;
        context->lineNumber = lineNumber;
        
        /* Copy line and clean it for parsing */
        strncpy(cleanLine, line, MAX_LINE_LENGTH - 1);
        cleanLine[MAX_LINE_LENGTH - 1] = '\0';
        
        /* Remove comments (start with ;) */
        char* comment = strchr(cleanLine, ';');
        if (comment) {
            *comment = '\0';
        }
        
        /* Remove leading/trailing whitespace */
        trim(cleanLine);
        
        /* Skip empty lines */
        if (cleanLine[0] == '\0') {
            fprintf(amFile, "%s", line); /* Copy as-is */
            continue;
        }
        
        /* Get first token to check for macro/mcroend */
        firstToken = my_strdup(cleanLine);
        if (!firstToken) {
            context->lineNumber = lineNumber;
            addError(context, "Memory allocation failed");
            return false;
        }
        
        token = my_strtok(firstToken, " \t", &saveptr);
        if (!token) {
            free(firstToken);
            fprintf(amFile, "%s", line); /* Copy as-is for blank lines */
            continue;
        }
        
        /* Check for macro definition start */
        if (strcmp(token, "mcro") == 0) {
            if (inMacroDefinition) {
                context->lineNumber = lineNumber;
                free(firstToken);
                addError(context, "Nested macro definitions are not allowed");
                return false;
            }
            
            /* Extract macro name */
            token = my_strtok(NULL, " \t", &saveptr);
            if (!token) {
                context->lineNumber = lineNumber;
                free(firstToken);
                addError(context, "Macro name missing");
                return false;
            }
            
            strncpy(macroName, token, MAX_LABEL_LENGTH - 1);
            macroName[MAX_LABEL_LENGTH - 1] = '\0';
            
            /* Start collecting macro content */
            inMacroDefinition = true;
            macroContent[0] = '\0'; /* Reset macro content */
            free(firstToken);
            continue;
        }
        
        /* Check for macro definition end */
        if (strcmp(token, "mcroend") == 0) {
            if (!inMacroDefinition) {
                context->lineNumber = lineNumber;
                free(firstToken);
                addError(context, "mcroend without matching mcro");
                return false;
            }
            
            /* Add macro to the table */
            if (!addMacro(context, macroName, macroContent)) {
                free(firstToken);
                return false;
            }
            
            inMacroDefinition = false;
            free(firstToken);
            continue;
        }
        
        free(firstToken); /* Done with firstToken */
        
        /* If inside macro definition, collect content */
        if (inMacroDefinition) {
            /* Append line to macro content */
            strcat(macroContent, line);
            continue;
        }
        
        /* Check if line is a macro call by getting the first token */
        firstToken = my_strdup(cleanLine);
        if (!firstToken) {
            context->lineNumber = lineNumber;
            addError(context, "Memory allocation failed");
            return false;
        }
        
        token = my_strtok(firstToken, " \t", &saveptr);
        if (token) {
            macro = findMacro(context, token);
            if (macro) {
                /* Expand macro */
                fprintf(amFile, "%s", macro->content);
                free(firstToken);
                continue;
            }
        }
        free(firstToken);
        
        /* Write line to output */
        fprintf(amFile, "%s", line);
    }
    
    /* Check if macro definition was properly closed */
    if (inMacroDefinition) {
        context->lineNumber = lineNumber;
        addError(context, "Unclosed macro definition");
        return false;
    }
    
    /* Reset the file positions */
    rewind(amFile);
    
    return true;
}
