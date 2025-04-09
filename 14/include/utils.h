#ifndef UTILS_H
#define UTILS_H

#include <stdlib.h>

/* Custom string utilities (C90 compatible) */
char* my_strdup(const char* str);
char* my_strtok(char* str, const char* delim, char** saveptr);
void my_snprintf(char* buffer, size_t size, const char* format, const char* str);

#endif /* UTILS_H */
