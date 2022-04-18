#include <stddef.h>
#include <stdint.h>
#include <sys/mandelbrot.h>

#define ITTERATIONS 80

#define ABS(x) ((x < 0) ? (-x) : x)

uint16_t width;
uint16_t height;
uint32_t *framebuffer;

uint64_t intsyscall(uint64_t id, uint64_t arg1, uint64_t arg2, uint64_t arg3,
                    uint64_t arg4, uint64_t arg5);

static inline double cos(double x) {
  register double val;
  asm volatile("fcos\n" : "=t"(val) : "0"(x));
  return val;
}

static inline double sin(double x) {
  register double val;
  asm volatile("fsin\n" : "=t"(val) : "0"(x));
  return val;
}

static inline double tan(double x) { return sin(x) / cos(x); }

static inline double sqrt(double x) {
  double res;
  asm("fsqrt" : "=t"(res) : "0"(x));
  return res;
}

void rect(size_t startx, size_t starty, size_t length, size_t height,
          uint32_t colour) {
  for (size_t x = startx; x < startx + length; x++)
    for (size_t y = starty; y < starty + height; y++)
      framebuffer[y * width + x] = colour;
}

uint32_t rgb2hex(uint8_t r, uint8_t g, uint8_t b) {
  return ((r & 0xff) << 16) + ((g & 0xff) << 8) + (b & 0xff);
}

void draw(double real, double imag) {
  for (int i = 0; i < height; i++) {
    for (int j = 0; j < width; j++) {
      double cx = 2.0 * ((j / (double)(width / 2)) - 1.0);
      double cy = 2.0 * ((i / (double)(height / 2)) - 1.0);

      double zx = cx;
      double zy = cy;

      int k = 0;

      while (k < ITTERATIONS && ABS(zx) < 2 && ABS(zy) < 2) {
        double nx = zx * zx - zy * zy + real;
        double ny = 2 * zx * zy + imag;

        zx = nx;
        zy = ny;

        k++;
      }

      if (k == ITTERATIONS)
        rect(j, i, 1, 1, 0);
      else {
        double a = k / 20.0;
        double b = a - 1.0;

        if (a > 1.0)
          a = 1.0;
        if (b < 0.0)
          b = 0.0;

        double col_r = (0 * (1 - a) + 0 * a) * (1 - b) + 255 * b;
        double col_g = (7 * (1 - a) + 255 * a) * (1 - b) + 255 * b;
        double col_b = (23 * (1 - a) + 127 * a) * (1 - b) + 255 * b;

        if (col_r > 255.0)
          col_r = 255.0;
        if (col_g > 255.0)
          col_g = 255.0;
        if (col_b > 255.0)
          col_b = 255.0;

        rect(j, i, 1, 1, rgb2hex(col_r, col_g, col_b));
      }
    }
  }
}

typedef struct mmap_args {
  void *addr;
  size_t length;
  uint64_t flags;
  uint64_t prot;
  size_t fd;
  size_t offset;
} mmap_args_t;

void main() {
  size_t fd = intsyscall(SYSCALL_OPEN, (uint64_t) "/dev/fb0", 0, 0, 0, 0);

  width =
    intsyscall(SYSCALL_IOCTL, fd, IOCTL_FBDEV_GET_WIDTH, (uint64_t)NULL, 0, 0);
  height =
    intsyscall(SYSCALL_IOCTL, fd, IOCTL_FBDEV_GET_HEIGHT, (uint64_t)NULL, 0, 0);
  size_t bpp =
    intsyscall(SYSCALL_IOCTL, fd, IOCTL_FBDEV_GET_HEIGHT, (uint64_t)NULL, 0, 0);

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

  // 4 really cool alorithms to choose from

  double a = 0;
  while (1) {
    draw(0.7885 * cos(a), 0.7885 * sin(a));
    a += 0.005;
  }

  /* double a = 0; */
  /* while (1) { */
  /* draw(0.7885 * tan(a), 0.7885 * sin(a)); */
  /* a += 0.005; */
  /* } */

  /* double a = 0; */
  /* while (1) { */
  /* draw(tan(a), tan(sin(a * 10) * 1)); */
  /* a += 0.0005; */
  /* } */

  /* double a = 0; */
  /* while (1) { */
  /* draw(tan(a), cos(a)/(1 + (1 / (1 + cos(a * a)) + ABS(tan(a * a))))); */
  /* a += 0.005; */
  /* } */
}
