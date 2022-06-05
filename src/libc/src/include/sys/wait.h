#ifndef __WAIT_H__
#define __WAIT_H__

#include <sys/types.h>

#define WNOHANG 1
#define WUNTRACED 2

#define WIFEXITED(s)	(!((s)&0xFF)
#define WIFSTOPPED(s) (((s)&0xFF) == 0x7F)
#define WEXITSTATUS(s) (((s) >> 8) & 0xFF)
#define WTERMSIG(s) ((s)&0x7F)
#define WSTOPSIG(s) (((s) >> 8) & 0xFF)
#define WIFSIGNALED(s) (((unsigned int)(s)-1 & 0xFFFF) < 0xFF)

pid_t wait(int *status);
pid_t waitpid(pid_t pid, int *status, int options);

#endif
