#include <fcntl.h>
#include <sys/mandelbrot.h>
#include <unistd.h>

char **environ;

int getpagesize() { return 0x1000; }

void _exit(int status) { intsyscall(SYSCALL_EXIT, status, 0, 0, 0, 0); }

int dup(int fildes) {
  return intsyscall(SYSCALL_FCNTL, fildes, F_DUPFD, 0, 0, 0);
}
