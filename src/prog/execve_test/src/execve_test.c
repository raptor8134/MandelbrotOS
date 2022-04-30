#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mandelbrot.h>
#include <sys/mman.h>
#include <sys/types.h>

#define ABS(x) ((x < 0) ? (-x) : x)

uint16_t width;
uint16_t height;
uint32_t *framebuffer;

uint64_t intsyscall(uint64_t id, uint64_t arg1, uint64_t arg2, uint64_t arg3,
                    uint64_t arg4, uint64_t arg5);

void swap(char *x, char *y) {
  char t = *x;
  *x = *y;
  *y = t;
}

int main(int argc, char *argv[], char *env[]) {
  intsyscall(SYSCALL_WRITE, 1, (uint64_t)argv[0], 3, 0, 0);
  intsyscall(SYSCALL_WRITE, 1, (uint64_t)argv[1], 3, 0, 0);
  intsyscall(SYSCALL_WRITE, 1, (uint64_t)argv[2], 3, 0, 0);
  (void)argc;
  (void)argv;
  (void)env;
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

  /* char c[50]; */
  /* itoa((int)(size_t)argv[0], c, 16); */
  /* intsyscall(SYSCALL_WRITE, 1, (uint64_t)c, strlen(c), 0, 0); */

  /* for (size_t i = 0; i < 500; i++) */
  /* { */
  /* intsyscall(SYSCALL_WRITE, 1, (uint64_t)&(((char *)(0x70000020000 -
   * 500))[i]), 1, 0, 0); */
  /* uint64_t val = ((uint64_t *)(0x70000020000 - 500 * sizeof(uint64_t)))[i];
   */
  /* char c[50]; */
  /* itoa(val, c, 16); */
  /* intsyscall(SYSCALL_WRITE, 1, (uint64_t)c, strlen(c), 0, 0); */
  /* intsyscall(SYSCALL_WRITE, 1, (uint64_t)" ", 1, 0, 0); */
  /* } */

  /* while (1) */
  /* ; */

  /* char *str = malloc(10); */
  /* str[0] = '\r'; */
  /* str[1] = '\n'; */
  /* str[2] = 'h'; */
  /* str[3] = 'i'; */
  /* str[4] = '\r'; */
  /* str[5] = '\n'; */
  /* str[6] = 0; */
  /* intsyscall(SYSCALL_WRITE, 1, (uint64_t)str, 6, 0, 0); */

  /* char *str = getenv("DING"); */
  /* if (str) */
  /* intsyscall(SYSCALL_WRITE, 1, (uint64_t)str, strlen(str), 0, 0); */
  /* else { */
  /* str = "No env var\r\n"; */
  /* intsyscall(SYSCALL_WRITE, 1, (uint64_t)str, strlen(str), 0, 0); */
  /* } */

  /* setenv("DING", "Hello test!\r\n", 0); */

  /* str = getenv("DING"); */
  /* if (str) */
  /* intsyscall(SYSCALL_WRITE, 1, (uint64_t)str, strlen(str), 0, 0); */
  /* else { */
  /* str = "No env var\r\n"; */
  /* intsyscall(SYSCALL_WRITE, 1, (uint64_t)str, strlen(str), 0, 0); */
  /* } */

  /* setenv("DING", "No overides??????\r\n", 0); */

  /* str = getenv("DING"); */
  /* if (str) */
  /* intsyscall(SYSCALL_WRITE, 1, (uint64_t)str, strlen(str), 0, 0); */
  /* else { */
  /* str = "No env var\r\n"; */
  /* intsyscall(SYSCALL_WRITE, 1, (uint64_t)str, strlen(str), 0, 0); */
  /* } */

  /* setenv("DING", "This time I do override\r\n", 1); */

  /* str = getenv("DING"); */
  /* if (str) */
  /* intsyscall(SYSCALL_WRITE, 1, (uint64_t)str, strlen(str), 0, 0); */
  /* else { */
  /* str = "No env var\r\n"; */
  /* intsyscall(SYSCALL_WRITE, 1, (uint64_t)str, strlen(str), 0, 0); */
  /* } */

  int pipefd[2];
  intsyscall(SYSCALL_PIPE, (uint64_t)pipefd, 0, 0, 0, 0);
  char *str = "Hello, world!\r\n";
  intsyscall(SYSCALL_WRITE, pipefd[1], (uint64_t)str, strlen(str) + 1, 0, 0);
  char c;
  while (((ssize_t)intsyscall(SYSCALL_READ, pipefd[0], (uint64_t)&c, 1, 0, 0)) >
         0)
    intsyscall(SYSCALL_WRITE, 1, (uint64_t)&c, 1, 0, 0);

  return 0;
}
