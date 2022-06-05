#include <errno.h>
#include <stdint.h>
#include <sys/mandelbrot.h>
#include <sys/stat.h>

int fstat(int fd, struct stat *statbuf) {
  int ret = intsyscall(SYSCALL_FSTAT, fd, (uint64_t)statbuf, 0, 0, 0);
  if (ret < 0) {
    errno = -ret;
    return -1;
  }
  return 0;
}
