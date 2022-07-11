#include "screen.h"
#include <math.h>
#include <stdint.h>
#include <stdio.h>

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
  int p, cache;
  double f;
  for (int x = 0; x < fb.width; x++) {
    f = (*function)(x - origin_x);
    y = floor(f) + origin_y;
    p = fb.width * (fb.height - y) + x;
    if (0 <= p && p < maxaddr && y < fb.height && !isnan(f)) {
      fb.buf[p] = color;
      cache = y;
      cache++;
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
double my_exp(int x) { return 10*exp(((double)x)/100); }

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
  graph_function(screen, &my_exp, 0xFF00FF, x, y);

  fb_close(screen);
}
