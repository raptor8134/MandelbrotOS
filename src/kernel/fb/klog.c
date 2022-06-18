#include <fb/fb.h>
#include <klog.h>
#include <lock.h>
#include <printf.h>
#include <stdarg.h>
#include <stdint.h>
#include <vprintf.h>

#define KLOG_GREEN 0x55FF55
#define KLOG_RED 0xFF5555
#define KLOG_YELLOW 0xFFFF55
#define KLOG_BLUE 0x5555FF

void klog(int type, char *message, ...) {
  va_list list;
  uint32_t old_fg = curr_fg_col;

  switch (type) {
    case 0:;
      printf("[ ");
      fb_set_fg(KLOG_GREEN);
      printf("OKAY");
      fb_set_fg(old_fg);
      printf(" ] ");
      va_start(list, message);
      vprintf(message, list);
      break;
    case 1:;
      printf("[ ");
      fb_set_fg(KLOG_RED);
      printf("FAIL");
      fb_set_fg(old_fg);
      printf(" ] ");
      va_start(list, message);
      vprintf(message, list);
      break;
    case 2:;
      printf("[ ");
      fb_set_fg(KLOG_YELLOW);
      printf("WARN");
      fb_set_fg(old_fg);
      printf(" ] ");
      va_start(list, message);
      vprintf(message, list);
      break;
    case 3:;
      printf("[ ");
      fb_set_fg(KLOG_BLUE);
      printf("INFO");
      fb_set_fg(old_fg);
      printf(" ] ");
      va_start(list, message);
      vprintf(message, list);
      break;
  }
  va_end(list);
}

int klog_init(int type, char *message) {
  uint32_t old_fg = curr_fg_col;

  switch (type) {
    case 0:;
      printf("[ ");
      fb_set_fg(KLOG_GREEN);
      printf("OKAY");
      fb_set_fg(old_fg);
      printf(" ] %s initialized \n", message);
      break;
    case 1:;
      printf("[ ");
      fb_set_fg(KLOG_RED);
      printf("FAIL");
      fb_set_fg(old_fg);
      printf(" ] %s not initialized \n", message);
      break;
  }

  return type;
}
