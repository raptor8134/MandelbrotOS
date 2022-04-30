#include <stddef.h>
#include <stdint.h>
#include <sys/mandelbrot.h>
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
  return (void *)intsyscall(SYSCALL_MMAP, (uint64_t)&args, 0, 0, 0, 0);
}

int munmap(void *addr, size_t length) {
  return intsyscall(SYSCALL_MUNMAP, (uint64_t)addr, length, 0, 0, 0);
}
