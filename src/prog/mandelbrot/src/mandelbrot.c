#include <stddef.h>
#include <stdint.h>
#include <sys/mandelbrot.h>

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

typedef struct pixel {
  uint8_t blue;
  uint8_t green;
  uint8_t red;
  uint8_t unused;
} pixel_t;

int floor(double x) {
  int xi = (int)x;
  return x < xi ? xi - 1 : xi;
}

pixel_t hsv2rgb(float h, float s, float v) {
  int i;
  float f, p, q, t;

  if (s > 1)
    s = 1;
  if (v > 1)
    v = 1;

  if (s == 0) {
    // achromatic (grey)
    return (pixel_t){
      .red = v * 255,
      .green = v * 255,
      .blue = v * 255,
    };
  }

  while (h >= 360)
    h -= 360;

  h /= 60; // sector 0 to 5
  i = floor(h);
  f = h - i; // factorial part of h
  p = v * (1 - s);
  q = v * (1 - s * f);
  t = v * (1 - s * (1 - f));
  switch (i) {
    case 0:
      return (pixel_t){
        .red = v * 255,
        .green = t * 255,
        .blue = p * 255,
      };
      break;
    case 1:
      return (pixel_t){
        .red = q * 255,
        .green = v * 255,
        .blue = p * 255,
      };
      break;
    case 2:
      return (pixel_t){
        .red = p * 255,
        .green = v * 255,
        .blue = t * 255,
      };
      break;
    case 3:
      return (pixel_t){
        .red = p * 255,
        .green = q * 255,
        .blue = v * 255,
      };
      break;
    case 4:
      return (pixel_t){
        .red = t * 255,
        .green = p * 255,
        .blue = v * 255,
      };
      break;
    default: // case 5:
      return (pixel_t){
        .red = v * 255,
        .green = p * 255,
        .blue = q * 255,
      };
      break;
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

uint32_t rgb2hex(pixel_t pix) {
  return (pix.red << 16) + (pix.green << 8) + pix.blue;
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

int main() {
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

  /* pixel_t *pix_buf = (void *)framebuffer; */

#define ORIG_SENS 0.02
#define ORIG_ZOOM 4
#define COLOUR_DEPTH 100

  int max = 1000;
  int threshold = 4;

  double posx = 0;
  double posy = 0;
  double zoom = ORIG_ZOOM;
  double sensitivity = ORIG_SENS;

  /* uint32_t colours[max]; */
  /* for (size_t i = 0; i < (size_t)max; i++) */
  /* colours[i] = */
  /* rgb2hex(hsv2rgb(((double)i / (double)max) * (360 - 280) + 280, 1, */
  /* (double)i / (double)(i + 5) + 0.1)); */
  /* colours[i] = */
  /* rgb2hex(hsv2rgb(((double)i / (double)max) * (360 - 300) + 0, 1, */
  /* (double)i / (double)(i + 5))); */
  /* colours[i] = rgb2hex(hsv2rgb(0, 0, (double)i / (double)COLOUR_DEPTH + */
  /* 0.1));  */
  /* { */

  /* double a = i / 20.0; */
  /* double b = a - 1.0; */

  /* if (a > 1.0) */
  /* a = 1.0; */
  /* if (b < 0.0) */
  /* b = 0.0; */

  /* double col_r = (0 * (1 - a) + 0 * a) * (1 - b) + 255 * b; */
  /* double col_g = (7 * (1 - a) + 255 * a) * (1 - b) + 255 * b; */
  /* double col_b = (23 * (1 - a) + 127 * a) * (1 - b) + 255 * b; */

  /* if (col_r > 255.0) */
  /* col_r = 255.0; */
  /* if (col_g > 255.0) */
  /* col_g = 255.0; */
  /* if (col_b > 255.0) */
  /* col_b = 255.0; */

  /* colours[i] = (uint32_t)col_r << 16 | (uint32_t)col_g << 8 |
   * (uint32_t)col_b; */
  /* } */

  while (1) {
    char c;
    intsyscall(SYSCALL_READ, 1, (uint64_t)&c, 1, 0, 0);

    switch (c) {
      case 'w':
        posy -= sensitivity;
        break;
      case 's':
        posy += sensitivity;
        break;
      case 'a':
        posx -= sensitivity;
        break;
      case 'd':
        posx += sensitivity;
        break;
      case 'z':
        zoom -= sensitivity;
        break;
      case 'x':
        zoom += sensitivity;
        break;
      case 'r':
        zoom = ORIG_ZOOM;
        sensitivity = ORIG_SENS;
        break;
      case 'c':
        posy = 0;
        posx = 0;
        break;
      default:
        continue;
    }

    sensitivity = zoom / 10;

    for (int row = 0; row < height; row++) {
      for (int col = 0; col < width; col++) {
        double c_re = (col - width / 2.0) * zoom / width + posx;
        double c_im = (row - height / 2.0) * zoom / width + posy;
        double x = 0, y = 0;
        double iteration = 0;
        /* threshold = zoom; */
        /* max = ABS((1 / zoom * 10) + 50); */
        /* max = 10 / zoom; */

        /* while (x * x + y * y <= threshold && iteration < max) { */
        /* double x_new = x * x - y * y + c_re; */
        /* y = 2 * x * y + c_im; */
        /* x = x_new; */
        /* iteration++; */
        /* } */

        while (x * x + y * y <= threshold && iteration < max) {
          double x_new = x * x - y * y + c_re;
          y = 2 * ABS(x * y) + c_im;
          x = x_new;
          iteration++;
        }

        if (iteration < max) {
          /* iteration = iteration + 1 - (log(log(sqrt(x * x + y * y))) /
           * log(2)); */
          /* iteration = iteration + 1 - (log((log(x * x + y * y) / 2) / log(2))
           * / log(2)); */

          /* double log_zn = log(x * x + y * y) / 2; */
          /* double nu = log(log_zn / log(2)) / log(2); */
          /* iteration = iteration + 1 - nu; */

          /* if (iteration < 0) */
          /* iteration = 0; */

          /* uint32_t colour1 = colours[floor(iteration)];  */
          /* uint32_t colour2 = colours[floor(iteration) + 1]; */
          /* uint32_t colour = lerp(colour1, colour2, iteration -
           * (int)iteration); */

          /* framebuffer[col + row * width] = colour; */
          /* framebuffer[col + row * width] = rgb2hex(hsv2rgb(((double)iteration
           * / (double)max) * (360 - 280) + 280, 1, (double)iteration /
           * (double)(iteration + 5) + 0.1)); */

          iteration = iteration + 1 - log(log(sqrt(x * x + y * y))) / log(2);

          /* framebuffer[col + row * width] = rgb2hex(hsv2rgb(((double)iteration
           * / (double)max) * (360 - 260) + 260, 1, (double)iteration /
           * (double)(iteration + 5) + 0.1)); */
          double a = iteration / 20.0;
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

          framebuffer[col + row * width] =
            rgb2hex((pixel_t){col_b, col_g, col_r, 0});
        } else
          framebuffer[col + row * width] = 0;
      }
    }
    for (size_t i = 0; i < 21; i++)
      framebuffer[height / 2 * width + (width / 2 - 10 + i)] ^= 0xffffff;
    for (size_t i = 0; i < 21; i++)
      framebuffer[(height / 2 - 10 + i) * width + width / 2] ^= 0xffffff;
  }
}
