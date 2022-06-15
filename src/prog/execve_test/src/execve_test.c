#include <fcntl.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mandelbrot.h>
#include <sys/mman.h>
#include <sys/types.h>

#define ABS(x) ((x < 0) ? (-x) : x)

uint16_t width;
uint16_t height;
uint32_t *framebuffer;

int main(int argc, char *argv[], char *env[]) {
  /* intsyscall(SYSCALL_WRITE, 1, (uint64_t)argv[0], 3, 0, 0); */
  /* intsyscall(SYSCALL_WRITE, 1, (uint64_t)argv[1], 3, 0, 0); */
  /* intsyscall(SYSCALL_WRITE, 1, (uint64_t)argv[2], 3, 0, 0); */
  (void)argc;
  (void)argv;
  (void)env;

  size_t fd = open("/dev/fb0", O_RDWR);

  width = ioctl(fd, IOCTL_FBDEV_GET_WIDTH, 0);
  height = ioctl(fd, IOCTL_FBDEV_GET_HEIGHT, 0);
  size_t bpp = ioctl(fd, IOCTL_FBDEV_GET_BPP, 0);

  framebuffer = (uint32_t *)mmap(NULL, width * height * (bpp / 8),
                                 PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);

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
  /* str[0] = ''; */
  /* str[1] = '\n'; */
  /* str[2] = 'h'; */
  /* str[3] = 'i'; */
  /* str[4] = ''; */
  /* str[5] = '\n'; */
  /* str[6] = 0; */
  /* intsyscall(SYSCALL_WRITE, 1, (uint64_t)str, 6, 0, 0); */

  /* char *str = getenv("DING"); */
  /* if (str) */
  /* intsyscall(SYSCALL_WRITE, 1, (uint64_t)str, strlen(str), 0, 0); */
  /* else { */
  /* str = "No env var\n"; */
  /* intsyscall(SYSCALL_WRITE, 1, (uint64_t)str, strlen(str), 0, 0); */
  /* } */

  /* setenv("DING", "Hello test!\n", 0); */

  /* str = getenv("DING"); */
  /* if (str) */
  /* intsyscall(SYSCALL_WRITE, 1, (uint64_t)str, strlen(str), 0, 0); */
  /* else { */
  /* str = "No env var\n"; */
  /* intsyscall(SYSCALL_WRITE, 1, (uint64_t)str, strlen(str), 0, 0); */
  /* } */

  /* setenv("DING", "No overides??????\n", 0); */

  /* str = getenv("DING"); */
  /* if (str) */
  /* intsyscall(SYSCALL_WRITE, 1, (uint64_t)str, strlen(str), 0, 0); */
  /* else { */
  /* str = "No env var\n"; */
  /* intsyscall(SYSCALL_WRITE, 1, (uint64_t)str, strlen(str), 0, 0); */
  /* } */

  /* setenv("DING", "This time I do override\n", 1); */

  /* str = getenv("DING"); */
  /* if (str) */
  /* intsyscall(SYSCALL_WRITE, 1, (uint64_t)str, strlen(str), 0, 0); */
  /* else { */
  /* str = "No env var\n"; */
  /* intsyscall(SYSCALL_WRITE, 1, (uint64_t)str, strlen(str), 0, 0); */
  /* } */

  int pipefd[2];
  intsyscall(SYSCALL_PIPE, (uint64_t)pipefd, 0, 0, 0, 0);
  /* char *str = "Hello, world!\n"; */
  char *str = "String\n";
  intsyscall(SYSCALL_WRITE, pipefd[1], (uint64_t)str, strlen(str) + 1, 0, 0);
  char c;
  while (((ssize_t)intsyscall(SYSCALL_READ, pipefd[0], (uint64_t)&c, 1, 0, 0)) >
         0)
    intsyscall(SYSCALL_WRITE, 1, (uint64_t)&c, 1, 0, 0);

  while (1)
    ;

  return 0;
}
