#ifdef __STDC__

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#else

typedef long FILE;
// stdio.h
int fprintf();
int printf();
int fopen();
int fclose();
int fseek();
#define SEEK_END 2
#define SEEK_SET 0
int ftell();
int fread();
extern int stdout;
extern int stderr;

// stdlib.h
int calloc();
long strtol(char *nptr, char **endptr, int base);
int exit();

#define NULL 0
// stddef.h stdio.h stdlib.h string.h time.h wchar.h
typedef int size_t;

#endif

// stdbool.h
typedef _Bool bool;
#define true 1
#define false 0

// ctype.h
int isalpha(int a);
int isspace(int c);

// string.h
size_t strlen(char *s);
int strncmp(char *s1, char *s2, size_t n);
char *strstr(char *haystack, char *needle);
void *memcpy(void *dest, void *src, size_t n);
