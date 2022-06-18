#include <fcntl.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/mandelbrot.h>
#include <sys/mman.h>
#include <unistd.h>

#define ABS(x) ((x < 0) ? (-x) : x)

uint16_t width;
uint16_t height;
uint32_t *framebuffer;

uint64_t intsyscall(uint64_t id, uint64_t arg1, uint64_t arg2, uint64_t arg3,
                    uint64_t arg4, uint64_t arg5);

int main() {
  size_t fd = open("/dev/fb0", O_RDWR);

  width = ioctl(fd, IOCTL_FBDEV_GET_WIDTH, 0);
  height = ioctl(fd, IOCTL_FBDEV_GET_HEIGHT, 0);
  size_t bpp = ioctl(fd, IOCTL_FBDEV_GET_BPP, 0);

  framebuffer = (uint32_t *)mmap(NULL, width * height * (bpp / 8),
                                 PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);

  /* size_t pid = intsyscall(SYSCALL_FORK, 0, 0, 0, 0, 0); */
  /* if (pid != 0) { */
  /* int exit = intsyscall(SYSCALL_WAITPID, pid, (uint64_t)NULL, 0, 0, 0); */
  /* (void)exit; */
  /* intsyscall(SYSCALL_WRITE, 1, (uint64_t) "Hello from parent thread!\n", */
  /* 27,  */
  /* 0, 0); */
  /* char str[] = "Child exit with  \n"; */
  /* str[16] = exit + '0'; */
  /* intsyscall(SYSCALL_WRITE, 1, (uint64_t)str, 19, 0, 0); */
  /* while (1) */
  /* ; */
  /* } else { */
  /* for (volatile size_t i = 0; i < 1000000000; i++) */
  /* asm volatile("nop"); */
  /* intsyscall(SYSCALL_WRITE, 1, (uint64_t) "Hello from child thread!\n", 26,
   */
  /* 0, 0); */
  /* } */

  char *file = "/prog/exectest";

  if (intsyscall(SYSCALL_ACCESS, (uint64_t)file, F_OK, 0, 0, 0) == (size_t)-1) {
    intsyscall(SYSCALL_WRITE, 1, (uint64_t) "Error: File does not exist!\n", 29,
               0, 0);
  }
  if (intsyscall(SYSCALL_ACCESS, (uint64_t)file, R_OK | X_OK, 0, 0, 0) ==
      (size_t)-1) {
    intsyscall(SYSCALL_WRITE, 1,
               (uint64_t) "Error: File cannot be exec'd or read!\n", 37, 0, 0);
  }

  int pid = fork();
  if (!pid) {
    printf("I am the child\n");
  } else {
    printf("I am the parent\n");
    while (1)
      ;
  }
  char *thing[4] = {"foo", "bar", "baz", 0};
  char *thin2[4] = {"PAT=\n/prog/dunc3", "ROOT=damn", "PATH=\nOVERHERE", 0};
  intsyscall(SYSCALL_EXEC, (uint64_t)file, (uint64_t)thing, (uint64_t)thin2, 0,
             0);

  return 9;
}
