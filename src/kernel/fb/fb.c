#include <boot/stivale2.h>
#include <fb/fb.h>
#include <font.h>
#include <lock.h>
#include <mm/pmm.h>
#include <stddef.h>
#include <stdint.h>

uint32_t *framebuffer;
uint16_t fb_height;
uint16_t fb_width;

uint32_t curr_fg_col = 0xffffff;
uint32_t curr_bg_col = 0x000000;

size_t curr_x = 0;
size_t curr_y = 0;

lock_t fb_lock = {0};

int init_fb(struct stivale2_struct_tag_framebuffer *framebuffer_info) {
  framebuffer = (uint32_t *)framebuffer_info->framebuffer_addr;
  fb_width = framebuffer_info->framebuffer_width;
  fb_height = framebuffer_info->framebuffer_height;
  return 0;
}

void fb_set_bg(uint32_t bg_col) {
  LOCK(fb_lock);
  curr_bg_col = bg_col;
  UNLOCK(fb_lock);
}

void fb_set_fg(uint32_t fg_col) {
  LOCK(fb_lock);
  curr_fg_col = fg_col;
  UNLOCK(fb_lock);
}

void fb_putpixel(size_t x, size_t y, uint32_t colour) {
  LOCK(fb_lock);
  framebuffer[y * fb_width + x] = colour;
  UNLOCK(fb_lock);
}

void putnc(int x, int y, char c, uint32_t fgc, uint32_t bgc) {
  for (int ly = 0; ly < FONT_HEIGHT; ly++)
    for (int lx = 0; lx < FONT_WIDTH; lx++) {
      uint8_t pixel = font_array[(c * FONT_CHAR_DEFINITION) + ly];
      if ((pixel >> lx) & 1)
        framebuffer[x + ((FONT_WIDTH - 1) - lx) + ((y + ly) * fb_width)] = fgc;
      else
        framebuffer[x + ((FONT_WIDTH - 1) - lx) + ((y + ly) * fb_width)] = bgc;
    }
}

void knewline() {
  for (size_t i = 0; i < (fb_width * fb_height); i++)
    framebuffer[i] = (framebuffer + fb_width * FONT_HEIGHT)[i];
  for (size_t i = 0; i < (fb_width * fb_height); i++)
    framebuffer[i] = (framebuffer + fb_width * FONT_HEIGHT)[i];

  curr_y -= FONT_HEIGHT;
  curr_x = 0;
}

void putc(char c, uint32_t fgc, uint32_t bgc) {
  switch (c) {
    case '\n':
      if (curr_y + FONT_HEIGHT * 2 > fb_height)
        knewline();
      else
        curr_y += FONT_HEIGHT;
    case '\r':
      curr_x = 0;
      break;
    case '\b':
      curr_x -= FONT_WIDTH;
      putnc(curr_x, curr_y, ' ', fgc, bgc);
      break;
    default:
      if (curr_x + FONT_WIDTH > fb_width) {
        curr_x = 0;
        if (curr_y + FONT_HEIGHT * 2 > fb_height)
          knewline();
        else
          curr_y += FONT_HEIGHT;
      }
      putnc(curr_x, curr_y, c, fgc, bgc);
      curr_x += FONT_WIDTH;
  }
}

void putchar(char c) { putc(c, curr_fg_col, curr_bg_col); }
