#ifndef __STDIO_H__
#define __STDIO_H__

struct FILE;

extern struct FILE *stdin;
extern struct FILE *stdout;
extern struct FILE *stderr;

typedef struct FILE {
  int fd;
} FILE;

__attribute__((format(printf, 1, 2))) int printf(const char *format, ...);
void perror(char *str);
char *fgets(char *s, int size, FILE *stream);
int remove(char *path);

#endif
