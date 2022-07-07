#include <fcntl.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/mandelbrot.h>
#include <sys/mman.h>
#include <unistd.h>

// lots of framebuffer stuff, maybe move to another file/library
typedef struct fb {
  uint32_t *buf;
  uint16_t width;
  uint16_t height;
  uint16_t bpp;
  int fd;
} fb_t;

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

void draw_axes(fb_t fb, uint32_t color, int x, int y) {
  for (int i = 0; i < fb.width; i++) {
    fb.buf[fb.width * (fb.height - y) + i] = color;
  }
  for (int i = 0; i < fb.height; i++) {
    fb.buf[fb.width * i + x] = color;
  }
}

// TODO add x and y dilation, fix spotty rendering
void graph_function(fb_t fb, double (*function)(int), uint32_t color,
                    int origin_x, int origin_y) {
  int maxaddr = fb.width * fb.height;
  long int y; // must be long to prevent overflow converting from double
  int p;
  double f;
  for (int x = 0; x < fb.width; x++) {
    f = (*function)(x - origin_x);
    y = floor(f) + origin_y;
    p = fb.width * (fb.height - y) + x;
    if (0 <= p && p < maxaddr && y < fb.height && !isnan(f)) {
      fb.buf[p] = color;
    }
  }
}

double my_cos(int x) { return 100 * cos(M_PI * x / 180); }
double my_tan(int x) { return 100 * tan(M_PI * x / 180); }
double my_log2(int x) { return 25 * log2(x); }
double my_polynomial(int x) {
  return ((x - 300) * (x - 100) * (x + 200)) / 100000;
}
double my_circle(int x) { return sqrt(200 * 200 - x * x); }

void main() {
  fb_t screen = fb_open("/dev/fb0");
  color_screen(screen, 0xFFFFFF);

  int x = screen.width / 2 - 200;
  int y = screen.height / 2 - 200;
  draw_axes(screen, 0x000000, x, y);

  graph_function(screen, &my_cos, 0xFF0000, x, y);
  graph_function(screen, &my_log2, 0x00FF00, x, y);
  graph_function(screen, &my_polynomial, 0x0000FF, x, y);
  graph_function(screen, &my_circle, 0x999999, x, y);

  fb_close(screen);
}
