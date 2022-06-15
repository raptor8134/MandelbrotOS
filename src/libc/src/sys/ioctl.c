#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include <sys/ioctl.h>
#include <sys/mandelbrot.h>

int ioctl(int d, int request,
          void *arg) { // TODO: This is not compliant to the standard
  int ret = intsyscall(SYSCALL_IOCTL, d, request, (uint64_t)arg, 0, 0);
  if (ret < 0) {
    errno = -ret;
    return -1;
  }
  return ret;
}
