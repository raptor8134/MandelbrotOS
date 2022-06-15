#include <fcntl.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/mandelbrot.h>
#include <sys/mman.h>
#include <sys/stat.h>

#define ABS(x) ((x < 0) ? (-x) : x)

uint16_t width;
uint16_t height;
uint32_t *framebuffer;

// Really nice and easy TGA reader gotten from
// https://wiki.osdev.org/Loading_Icons
uint32_t *tga_parse(unsigned char *ptr, int size) {
  int i, j, k, x, y, w = (ptr[13] << 8) + ptr[12], h = (ptr[15] << 8) + ptr[14],
                     o = (ptr[11] << 8) + ptr[10];
  int m = ((ptr[1] ? (ptr[7] >> 3) * ptr[5] : 0) + 18);

  if (w < 1 || h < 1)
    return NULL;

  uint32_t *data = malloc((w * h + 2) * sizeof(uint32_t));
  if (!data)
    return NULL;

  switch (ptr[2]) {
    case 1:
      if (ptr[6] != 0 || ptr[4] != 0 || ptr[3] != 0 ||
          (ptr[7] != 24 && ptr[7] != 32)) {
        free(data);
        return NULL;
      }
      for (y = i = 0; y < h; y++) {
        k = ((!o ? h - y - 1 : y) * w);
        for (x = 0; x < w; x++) {
          j = ptr[m + k++] * (ptr[7] >> 3) + 18;
          data[2 + i++] = ((ptr[7] == 32 ? ptr[j + 3] : 0xFF) << 24) |
                          (ptr[j + 2] << 16) | (ptr[j + 1] << 8) | ptr[j];
        }
      }
      break;
    case 2:
      if (ptr[5] != 0 || ptr[6] != 0 || ptr[1] != 0 ||
          (ptr[16] != 24 && ptr[16] != 32)) {
        free(data);
        return NULL;
      }
      for (y = i = 0; y < h; y++) {
        j = ((!o ? h - y - 1 : y) * w * (ptr[16] >> 3));
        for (x = 0; x < w; x++) {
          data[2 + i++] =
            (ptr[j + 2] << 16) | (ptr[j + 1] << 8) | (ptr[j] << 0);
          j += ptr[16] >> 3;
        }
      }
      break;
    case 9:
      if (ptr[6] != 0 || ptr[4] != 0 || ptr[3] != 0 ||
          (ptr[7] != 24 && ptr[7] != 32)) {
        free(data);
        return NULL;
      }
      y = i = 0;
      for (x = 0; x < w * h && m < size;) {
        k = ptr[m++];
        if (k > 127) {
          k -= 127;
          x += k;
          j = ptr[m++] * (ptr[7] >> 3) + 18;
          while (k--) {
            if (!(i % w)) {
              i = ((!o ? h - y - 1 : y) * w);
              y++;
            }
            data[2 + i++] = ((ptr[7] == 32 ? ptr[j + 3] : 0xFF) << 24) |
                            (ptr[j + 2] << 16) | (ptr[j + 1] << 8) | ptr[j];
          }
        } else {
          k++;
          x += k;
          while (k--) {
            j = ptr[m++] * (ptr[7] >> 3) + 18;
            if (!(i % w)) {
              i = ((!o ? h - y - 1 : y) * w);
              y++;
            }
            data[2 + i++] = ((ptr[7] == 32 ? ptr[j + 3] : 0xFF) << 24) |
                            (ptr[j + 2] << 16) | (ptr[j + 1] << 8) | ptr[j];
          }
        }
      }
      break;
    case 10:
      if (ptr[5] != 0 || ptr[6] != 0 || ptr[1] != 0 ||
          (ptr[16] != 24 && ptr[16] != 32)) {
        free(data);
        return NULL;
      }
      y = i = 0;
      for (x = 0; x < w * h && m < size;) {
        k = ptr[m++];
        if (k > 127) {
          k -= 127;
          x += k;
          while (k--) {
            if (!(i % w)) {
              i = ((!o ? h - y - 1 : y) * w);
              y++;
            }
            data[2 + i++] = ((ptr[16] == 32 ? ptr[m + 3] : 0xFF) << 24) |
                            (ptr[m + 2] << 16) | (ptr[m + 1] << 8) | ptr[m];
          }
          m += ptr[16] >> 3;
        } else {
          k++;
          x += k;
          while (k--) {
            if (!(i % w)) {
              i = ((!o ? h - y - 1 : y) * w);
              y++;
            }
            data[2 + i++] = ((ptr[16] == 32 ? ptr[m + 3] : 0xFF) << 24) |
                            (ptr[m + 2] << 16) | (ptr[m + 1] << 8) | ptr[m];
            m += ptr[16] >> 3;
          }
        }
      }
      break;
  }
  data[0] = w;
  data[1] = h;
  return data;
}

int main() {
  size_t fd = open("/dev/fb0", O_RDWR);

  width = ioctl(fd, IOCTL_FBDEV_GET_WIDTH, 0);
  height = ioctl(fd, IOCTL_FBDEV_GET_HEIGHT, 0);
  size_t bpp = ioctl(fd, IOCTL_FBDEV_GET_BPP, 0);

  framebuffer = (uint32_t *)mmap(NULL, width * height * (bpp / 8),
                                 PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);

  int fd2 =
    intsyscall(SYSCALL_OPEN, (uint64_t) "/data/glenda.tga", O_RDWR, 0, 0, 0);

  struct stat stat;
  fstat(fd2, &stat);
  size_t file_size = stat.st_size;

  uint8_t *buffer = malloc(file_size);
  intsyscall(SYSCALL_READ, fd2, (uint64_t)buffer, file_size, 0, 0);

  uint32_t *img = tga_parse(buffer, file_size);

  for (size_t i = 0; i < img[0]; i++)
    for (size_t j = 0; j < img[1]; j++)
      framebuffer[(img[1] - 1 - j) * width + i] = img[j * img[0] + i + 2];

  return 0;
}
