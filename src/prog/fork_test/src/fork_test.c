#include <stddef.h>
#include <stdint.h>
#include <sys/mandelbrot.h>
#include <sys/mman.h>

#define ABS(x) ((x < 0) ? (-x) : x)

uint16_t width;
uint16_t height;
uint32_t *framebuffer;

uint64_t intsyscall(uint64_t id, uint64_t arg1, uint64_t arg2, uint64_t arg3,
                    uint64_t arg4, uint64_t arg5);

int main() {
  intsyscall(SYSCALL_WRITE, 1, (uint64_t) "Hello from child thread!\r\n", 26, 0,
             0);
  size_t fd = intsyscall(SYSCALL_OPEN, (uint64_t) "/dev/fb0", 0, 0, 0, 0);

  width =
    intsyscall(SYSCALL_IOCTL, fd, IOCTL_FBDEV_GET_WIDTH, (uint64_t)NULL, 0, 0);
  height =
    intsyscall(SYSCALL_IOCTL, fd, IOCTL_FBDEV_GET_HEIGHT, (uint64_t)NULL, 0, 0);
  size_t bpp =
    intsyscall(SYSCALL_IOCTL, fd, IOCTL_FBDEV_GET_BPP, (uint64_t)NULL, 0, 0);

  mmap_args_t args = (mmap_args_t){
    .addr = NULL,
    .fd = fd,
    .length = width * height * (bpp / 8),
    .offset = 0,
    .prot = PROT_READ | PROT_WRITE,
    .flags = 0,
  };

  framebuffer =
    (uint32_t *)intsyscall(SYSCALL_MMAP, (uint64_t)&args, 0, 0, 0, 0);

  /* size_t pid = intsyscall(SYSCALL_FORK, 0, 0, 0, 0, 0); */
  /* if (pid != 0) { */
  /* int exit = intsyscall(SYSCALL_WAITPID, pid, (uint64_t)NULL, 0, 0, 0); */
  /* (void)exit; */
  /* intsyscall(SYSCALL_WRITE, 1, (uint64_t) "Hello from parent thread!\r\n", */
  /* 27,  */
  /* 0, 0); */
  /* char str[] = "Child exit with  \r\n"; */
  /* str[16] = exit + '0'; */
  /* intsyscall(SYSCALL_WRITE, 1, (uint64_t)str, 19, 0, 0); */
  /* while (1) */
  /* ; */
  /* } else { */
  /* for (volatile size_t i = 0; i < 1000000000; i++) */
  /* asm volatile("nop"); */
  /* intsyscall(SYSCALL_WRITE, 1, (uint64_t) "Hello from child thread!\r\n", 26,
   */
  /* 0, 0); */
  /* } */

  char *file = "/prog/exectest";

  if (intsyscall(SYSCALL_ACCESS, (uint64_t)file, F_OK, 0, 0, 0) == (size_t)-1) {
    intsyscall(SYSCALL_WRITE, 1, (uint64_t) "Error: File does not exist!\r\n",
               29, 0, 0);
  }
  if (intsyscall(SYSCALL_ACCESS, (uint64_t)file, R_OK | X_OK, 0, 0, 0) ==
      (size_t)-1) {
    intsyscall(SYSCALL_WRITE, 1,
               (uint64_t) "Error: File cannot be exec'd or read!\r\n", 37, 0,
               0);
  }

  char *thing[4] = {"foo", "bar", "baz", 0};
  char *thin2[4] = {"PAT=\r\n/prog/dunc3", "ROOT=damn", "PATH=\r\nOVERHERE", 0};
  intsyscall(SYSCALL_EXEC, (uint64_t)file, (uint64_t)thing, (uint64_t)thin2, 0,
             0);

  return 9;
}
