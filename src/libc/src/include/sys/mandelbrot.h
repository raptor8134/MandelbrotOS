#ifndef __SYS_MANDELBROT_H__
#define __SYS_MANDELBROT_H__

#include <stddef.h>
#include <stdint.h>

#define R_OK 0x4
#define W_OK 0x2
#define X_OK 0x1
#define F_OK 0x0

#define O_RDONLY 0x1
#define O_WRONLY 0x2
#define O_RDWR 0x4
#define O_CREAT 0x200
#define O_DIRECTORY 0x200000
#define O_EXEC 0x400000
#define O_ACCMODE (O_RDONLY | O_WRONLY | O_RDWR)

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

#define IOCTL_FBDEV_GET_WIDTH 1
#define IOCTL_FBDEV_GET_HEIGHT 2
#define IOCTL_FBDEV_GET_BPP 3

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

typedef struct mmap_args {
  void *addr;
  size_t length;
  uint64_t flags;
  uint64_t prot;
  size_t fd;
  size_t offset;
} mmap_args_t;

uint64_t intsyscall(uint64_t id, uint64_t arg_1, uint64_t arg_2, uint64_t arg_3,
                    uint64_t arg_4, uint64_t arg_5);

#endif
