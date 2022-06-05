#include <fcntl.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/ioctl.h>
#include <sys/mandelbrot.h>
#include <sys/mman.h>

#define ITTERATIONS 90

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

/* origin: FreeBSD /usr/src/lib/msun/src/e_log.c */
/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunSoft, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 */

static const double ln2_hi = 6.93147180369123816490e-01,
                    ln2_lo = 1.90821492927058770002e-10,
                    Lg1 = 6.666666666666735130e-01,
                    Lg2 = 3.999999999940941908e-01,
                    Lg3 = 2.857142874366239149e-01,
                    Lg4 = 2.222219843214978396e-01,
                    Lg5 = 1.818357216161805012e-01,
                    Lg6 = 1.531383769920937332e-01,
                    Lg7 = 1.479819860511658591e-01;

double log(double x) {
  union {
    double f;
    uint64_t i;
  } u = {x};
  double hfsq, f, s, z, R, w, t1, t2, dk;
  uint32_t hx;
  int k;

  hx = u.i >> 32;
  k = 0;
  if (hx < 0x00100000 || hx >> 31) {
    if (u.i << 1 == 0)
      return -1 / (x * x);
    if (hx >> 31)
      return (x - x) / 0.0;

    k -= 54;
    x *= 0x1p54;
    u.f = x;
    hx = u.i >> 32;
  } else if (hx >= 0x7ff00000) {
    return x;
  } else if (hx == 0x3ff00000 && u.i << 32 == 0)
    return 0;

  hx += 0x3ff00000 - 0x3fe6a09e;
  k += (int)(hx >> 20) - 0x3ff;
  hx = (hx & 0x000fffff) + 0x3fe6a09e;
  u.i = (uint64_t)hx << 32 | (u.i & 0xffffffff);
  x = u.f;

  f = x - 1.0;
  hfsq = 0.5 * f * f;
  s = f / (2.0 + f);
  z = s * s;
  w = z * z;
  t1 = w * (Lg2 + w * (Lg4 + w * Lg6));
  t2 = z * (Lg1 + w * (Lg3 + w * (Lg5 + w * Lg7)));
  R = t2 + t1;
  dk = k;
  return s * (hfsq + R) + dk * ln2_lo - hfsq + f + dk * ln2_hi;
}

void rect(size_t startx, size_t starty, size_t length, size_t height,
          uint32_t colour) {
  for (size_t x = startx; x < startx + length; x++)
    for (size_t y = starty; y < starty + height; y++)
      framebuffer[y * width + x] = colour;
}

static inline uint32_t rgb2hex(uint8_t r, uint8_t g, uint8_t b) {
  return ((r & 0xff) << 16) + ((g & 0xff) << 8) + (b & 0xff);
}

void draw(double real, double imag) {
  for (int i = 0; i < height; i++) {
    for (int j = 0; j < width; j++) {
      double cx = 2.0 * ((j / (double)((double)width / 2)) - 1.0);
      double cy = 2.0 * ((i / (double)((double)height / 2)) - 1.0);

      double zx = cx;
      double zy = cy;

      int k = 0;
      int threshold = 100;
      double smooth_colour = 0;

      while (k < ITTERATIONS && zx * zx + zy * zy <= threshold) {
        double nx = zx * zx - zy * zy + real;
        double ny = 2 * zx * zy + imag;

        zx = nx;
        zy = ny;

        k++;
      }

      smooth_colour = k + 2 -
                      log(log(zx * zx + zy * zy)) /
                        0.69314718055994528622676398299518041312694549560546875;

      if (k == ITTERATIONS)
        framebuffer[i * width + j] = 0;
      /* rect(j, i, 1, 1, 0); */
      else {
        double a = smooth_colour / 30.0;
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

        /* rect(j, i, 1, 1, rgb2hex(col_r, col_g, col_b)); */
        framebuffer[i * width + j] = rgb2hex(col_r, col_g, col_b);
      }
    }
  }
}

void main() {
  size_t fd = open("/dev/fb0", O_RDWR);

  width = ioctl(fd, IOCTL_FBDEV_GET_WIDTH, 0);
  height = ioctl(fd, IOCTL_FBDEV_GET_HEIGHT, 0);
  size_t bpp = ioctl(fd, IOCTL_FBDEV_GET_BPP, 0);

  framebuffer = (uint32_t *)mmap(NULL, width * height * (bpp / 8),
                                 PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);

  // 4 really cool alorithms to choose from

  double a = 0;
  while (1) {
    draw(0.7885 * cos(a), 0.7885 * sin(a));
    a += 0.004;
  }

  /* double a = 0; */
  /* while (1) { */
  /* draw(0.7182818284590452353602874713527 * cos(a), */
  /* 0.7182818284590452353602874713527 * sin(a)); */
  /* a += 0.004; */
  /* } */

  /* double a = 0; */
  /* while (1) { */
  /* draw(0.7885 * tan(a), 0.7885 * sin(a)); */
  /* a += 0.01; */
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
