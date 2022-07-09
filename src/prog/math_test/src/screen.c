#include "screen.h"
#include <fcntl.h>
#include <stdint.h>
#include <sys/ioctl.h>
#include <sys/mandelbrot.h>
#include <sys/mman.h>
#include <unistd.h>

fb_t fb_open(char *path) {
  fb_t ret;
  ret.fd = open(path, O_RDWR);
  ret.width = ioctl(ret.fd, IOCTL_FBDEV_GET_WIDTH, 0);
  ret.height = ioctl(ret.fd, IOCTL_FBDEV_GET_HEIGHT, 0);
  ret.bpp = ioctl(ret.fd, IOCTL_FBDEV_GET_BPP, 0);
  ret.buf = (uint32_t *)mmap(NULL, ret.width * ret.height * (ret.bpp / 8),
                             PROT_READ | PROT_WRITE, MAP_PRIVATE, ret.fd, 0);
  return ret;
}

void fb_close(fb_t screen) { close(screen.fd); }

void color_screen(fb_t fb, uint32_t color) {
  int maxaddr = fb.width * fb.height;
  for (int i = 0; i < maxaddr; i++) {
    fb.buf[i] = color;
  }
}
