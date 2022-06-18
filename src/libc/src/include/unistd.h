#ifndef __UNISTD_H__
#define __UNISTD_H__

#include <stddef.h>
#include <sys/types.h>

#define STDIN_FILENO 0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2

#define R_OK 0x4
#define W_OK 0x2
#define X_OK 0x1
#define F_OK 0x0

extern char **environ;

int getpagesize();
void _exit(int status);
int dup(int fildes);
int access(char *pathname, int mode);
pid_t fork();
int pipe(int pipefd[2]);
int close(int fd);
int execve(char *filename, char *argv[], char *envp[]);
ssize_t read(int fd, void *buf, size_t count);
ssize_t write(int fd, void *buf, size_t count);

#endif
