#ifndef __STDLIB_H__
#define __STDLIB_H__

#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1

void _Exit(int status);
void abort();
int abs(int i);
double atof(char *arr);
int atoi(char *p);
long int atol(char *p);
long long int atoll(char *p);

#endif
