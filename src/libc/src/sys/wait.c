#include <errno.h>
#include <stdint.h>
#include <sys/mandelbrot.h>
#include <sys/types.h>
#include <sys/wait.h>

pid_t waitpid(pid_t pid, int *status, int options) {
  pid_t ret = intsyscall(SYSCALL_WAITPID, pid, (uint64_t)status, options, 0, 0);
  if (ret < 0) {
    errno = -ret;
    return -1;
  }
  return ret;
}

pid_t wait(int *status) { return waitpid(-1, status, 0); }
