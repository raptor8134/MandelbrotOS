#include <sys/mandelbrot.h>
#include <unistd.h>

void _exit(int status) { intsyscall(SYSCALL_EXIT, status, 0, 0, 0, 0); }
