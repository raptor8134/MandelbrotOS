#include <errno.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/mandelbrot.h>
#include <sys/mman.h>
#include <sys/types.h>

void *mmap(void *addr, size_t length, int prot, int flags, int fd,
           off_t offset) {
  mmap_args_t args = (mmap_args_t){
    .addr = addr,
    .length = length,
    .prot = prot,
    .flags = flags,
    .fd = fd,
    .offset = offset,
  };
  void *ret = (void *)intsyscall(SYSCALL_MMAP, (uint64_t)&args, 0, 0, 0, 0);
  if ((int64_t)ret < (int64_t)0) {
    errno = -(int64_t)ret;
    return MAP_FAILED;
  }
  return ret;
}

int munmap(void *addr, size_t length) {
  int ret = intsyscall(SYSCALL_MUNMAP, (uint64_t)addr, length, 0, 0, 0);
  if (ret < 0) {
    errno = -ret;
    return -1;
  }
  return 0;
}
