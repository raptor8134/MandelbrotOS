#ifndef __SYSCALL_H__
#define __SYSCALL_H__

#include <fs/vfs.h>
#include <stdint.h>

#define SYSCALL_OPEN 0
#define SYSCALL_CLOSE 1
#define SYSCALL_READ 2
#define SYSCALL_WRITE 3
#define SYSCALL_EXEC 4
#define SYSCALL_MMAP 5
#define SYSCALL_MUNMAP 6
#define SYSCALL_STAT 7
#define SYSCALL_FSTAT 8
#define SYSCALL_GETPID 9
#define SYSCALL_EXIT 10
#define SYSCALL_FORK 11
#define SYSCALL_GETTIMEOFDAY 12
#define SYSCALL_FSYNC 13
#define SYSCALL_IOCTL 14
#define SYSCALL_GETPPID 15
#define SYSCALL_SEEK 16
#define SYSCALL_WAITPID 17
#define SYSCALL_ACCESS 18
#define SYSCALL_PIPE 19
#define SYSCALL_FCNTL 20
#define SYSCALL_REMOVE 21

#define PROT_READ 0x1
#define PROT_WRITE 0x2
#define PROT_EXEC 0x4
#define PROT_NONE 0x0

#define MAP_PRIVATE 0x1
#define MAP_SHARED 0x2
#define MAP_FIXED 0x4
#define MAP_ANON 0x8

#define MMAP_MAX_SIZE 0x2000000

typedef struct mmap_args {
  void *addr;
  size_t length;
  uint64_t flags;
  uint64_t prot;
  int fd;
  size_t offset;
} mmap_args_t;

typedef struct syscall_file {
  fs_file_t *file;
  int flags;
} syscall_file_t;

int init_syscalls();

#endif
