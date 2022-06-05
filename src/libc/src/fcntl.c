#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <sys/mandelbrot.h>
#include <sys/types.h>

int open(char *pathname, int flags, ...) {
  if (flags & O_CREAT) {
    va_list v;
    va_start(v, flags);
    mode_t mode = va_arg(v, mode_t);
    va_end(v);

    int ret = intsyscall(SYSCALL_OPEN, (uint64_t)pathname, flags, mode, 0, 0);
    if (ret < 0) {
      errno = -ret;
      return -1;
    }
    return ret;
  } else {
    int ret = intsyscall(SYSCALL_OPEN, (uint64_t)pathname, flags, 0, 0, 0);
    if (ret < 0) {
      errno = -ret;
      return -1;
    }
    return ret;
  }
}
