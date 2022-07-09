#ifndef __SCREEN_H__
#define __SCREEN_H__

#include <stdint.h>

typedef struct fb {
  uint32_t *buf;
  uint16_t width;
  uint16_t height;
  uint16_t bpp;
  int fd;
} fb_t;

fb_t fb_open(char *path);
void fb_close(fb_t screen);
void color_screen(fb_t screen, uint32_t color);

#endif
