#ifndef __STDLIB_H__
#define __STDLIB_H__

#include <stddef.h>

#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1

void _Exit(int status);
void exit(int status);
void abort();
int abs(int i);
double atof(char *arr);
int atoi(char *p);
long int atol(char *p);
long long int atoll(char *p);
void *malloc(size_t size);
void free(void *ptr);
void *calloc(size_t size, size_t size2);
void *realloc(void *ptr, size_t size);
char *getenv(char *name);
int setenv(char *name, char *value, int overide);
int putenv(char *name);

#endif
