#include <stdint.h>
#include <sys/mandelbrot.h>
#include <sys/stat.h>

int fstat(int fd, struct stat *statbuf) {
  return intsyscall(SYSCALL_FSTAT, fd, (uint64_t)statbuf, 0, 0, 0);
}
