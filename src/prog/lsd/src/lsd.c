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

void main() {
  size_t fd = open("/dev/fb0", O_RDWR);

  width = ioctl(fd, IOCTL_FBDEV_GET_WIDTH, 0);
  height = ioctl(fd, IOCTL_FBDEV_GET_HEIGHT, 0);
  size_t bpp = ioctl(fd, IOCTL_FBDEV_GET_BPP, 0);

  framebuffer = (uint32_t *)mmap(NULL, width * height * (bpp / 8),
                                 PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);

  pixel_t *fb_pix = (void *)framebuffer;

  size_t current_offset = 0;

  while (1) {
    for (size_t x = 0; x < width; x++) {
      for (size_t y = 0; y < height; y++) {
        size_t raw_pos = x + y * width;
        fb_pix[raw_pos].blue =
          (x * 4 + current_offset) ^ (y * 4 + current_offset);
        fb_pix[raw_pos].red =
          (y * 1 + current_offset) ^ (x * 1 + current_offset);
        fb_pix[raw_pos].green =
          (y * 2 + current_offset) ^ (x * 2 + current_offset);

        /* fb_pix[raw_pos].blue = y + current_offset; */
        /* fb_pix[raw_pos].red = x + current_offset; */
        /* fb_pix[raw_pos].green = (y & x) + current_offset; */

        /* fb_pix[raw_pos].blue = y + current_offset; */
        /* fb_pix[raw_pos].red = x + current_offset; */
        /* fb_pix[raw_pos].green = y * x + current_offset; */

        /* fb_pix[raw_pos].blue = x; */
        /* fb_pix[raw_pos].red = y; */
        /* fb_pix[raw_pos].green = current_offset; */

        /* framebuffer[raw_pos] = current_offset++; */
      }
    }

    current_offset += 1;

    /* if (current_offset < 5) { */
    /* reverse = 0; */
    /* current_offset++; */
    /* } */
    /* else if (reverse) */
    /* current_offset -= 1; */
    /* else if (current_offset < 0xff) */
    /* current_offset += 1; */
    /* else  */
    /* reverse = 1; */
  }
}
