#ifndef __UNISTD_H__
#define __UNISTD_H__

extern char **environ;

int getpagesize();
void _exit(int status);
int dup(int fildes);

#endif
