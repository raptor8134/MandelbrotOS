#include <fcntl.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/ioctl.h>
#include <sys/mandelbrot.h>
#include <sys/mman.h>

uint16_t width;
uint16_t height;
uint32_t *framebuffer;

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
  if (s == 0) {
    // achromatic (grey)
    return (pixel_t){
      .red = s,
      .green = s,
      .blue = s,
    };
  }
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

void main() {
  size_t fd = open("/dev/fb0", O_RDWR);

  width = ioctl(fd, IOCTL_FBDEV_GET_WIDTH, 0);
  height = ioctl(fd, IOCTL_FBDEV_GET_HEIGHT, 0);
  size_t bpp = ioctl(fd, IOCTL_FBDEV_GET_BPP, 0);

  framebuffer = (uint32_t *)mmap(NULL, width * height * (bpp / 8),
                                 PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);

  pixel_t *fb_pix = (void *)framebuffer;

  double p = 0;

  while (1) {
    for (size_t x = 0; x < width; x++) {
      for (size_t y = 0; y < height; y++) {
        size_t raw_pos = x + y * width;
        fb_pix[raw_pos] = hsv2rgb(p, 1 - ((double)y / (double)height),
                                  (double)x / (double)width);
      }
      p += 0.0005;
      if (p > 360.0)
        p = 0;
    }
  }
}
