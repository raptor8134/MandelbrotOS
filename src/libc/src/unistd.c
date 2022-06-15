#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <sys/mandelbrot.h>
#include <sys/types.h>
#include <unistd.h>

char **environ;

int getpagesize() {
  return 0x1000;
} // TODO: maybe make the kernel give this back?

void _exit(int status) { intsyscall(SYSCALL_EXIT, status, 0, 0, 0, 0); }

int dup(int fildes) {
  int ret = intsyscall(SYSCALL_FCNTL, fildes, F_DUPFD, 0, 0, 0);
  if (ret < 0) {
    errno = -ret;
    return -1;
  }
  return ret;
}

int access(char *pathname, int mode) {
  int ret = intsyscall(SYSCALL_ACCESS, (uint64_t)pathname, mode, 0, 0, 0);
  if (ret < 0) {
    errno = -ret;
    return -1;
  }
  return ret;
}

pid_t fork() {
  pid_t ret = intsyscall(SYSCALL_FORK, 0, 0, 0, 0, 0);
  if (ret < 0) {
    errno = -ret;
    return -1;
  }
  return ret;
}

int pipe(int pipefd[2]) {
  int ret = intsyscall(SYSCALL_PIPE, (uint64_t)pipefd, 0, 0, 0, 0);
  if (ret < 0) {
    errno = -ret;
    return -1;
  }
  return ret;
}

int close(int fd) {
  int ret = intsyscall(SYSCALL_CLOSE, fd, 0, 0, 0, 0);
  if (ret < 0) {
    errno = -ret;
    return -1;
  }
  return ret;
}

int execve(char *filename, char *argv[], char *envp[]) {
  int ret = intsyscall(SYSCALL_EXEC, (uint64_t)filename, (uint64_t)argv,
                       (uint64_t)envp, 0, 0);
  if (ret < 0) {
    errno = -ret;
    return -1;
  }
  return ret;
}

ssize_t read(int fd, void *buf, size_t count) {
  int ret = intsyscall(SYSCALL_READ, fd, (uint64_t)buf, count, 0, 0);
  if (ret < 0) {
    errno = -ret;
    return -1;
  }
  return ret;
}

ssize_t write(int fd, void *buf, size_t count) {
  int ret = intsyscall(SYSCALL_WRITE, fd, (uint64_t)buf, count, 0, 0);
  if (ret < 0) {
    errno = -ret;
    return -1;
  }
  return ret;
}
