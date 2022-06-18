#include <lock.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include <fb/fb.h>
#include <lock.h>
#include <printf.h>
#include <string.h>
#include <vprintf.h>

static lock_t printf_lock = {0};

// Not my implementation. Taken from https://wiki.osdev.org/User:A22347/Printf

char *__int_str(intmax_t i, char b[], int base, bool plusSignIfNeeded,
                bool spaceSignIfNeeded, int paddingNo, bool justify,
                bool zeroPad) {

  char digit[32] = {0};
  memset(digit, 0, 32);
  strcpy(digit, "0123456789");

  if (base == 16) {
    strcat(digit, "ABCDEF");
  } else if (base == 17) {
    strcat(digit, "abcdef");
    base = 16;
  }

  char *p = b;
  if (i < 0) {
    *p++ = '-';
    i *= -1;
  } else if (plusSignIfNeeded) {
    *p++ = '+';
  } else if (!plusSignIfNeeded && spaceSignIfNeeded) {
    *p++ = ' ';
  }

  intmax_t shifter = i;
  do {
    ++p;
    shifter = shifter / base;
  } while (shifter);

  *p = '\0';
  do {
    *--p = digit[i % base];
    i = i / base;
  } while (i);

  int padding = paddingNo - (int)strlen(b);
  if (padding < 0)
    padding = 0;

  if (justify) {
    while (padding--) {
      if (zeroPad) {
        b[strlen(b)] = '0';
      } else {
        b[strlen(b)] = ' ';
      }
    }

  } else {
    char a[256] = {0};
    while (padding--) {
      if (zeroPad) {
        a[strlen(a)] = '0';
      } else {
        a[strlen(a)] = ' ';
      }
    }
    strcat(a, b);
    strcpy(b, a);
  }

  return b;
}

char *__uint_str(uintmax_t i, char b[], int base, bool plusSignIfNeeded,
                 bool spaceSignIfNeeded, int paddingNo, bool justify,
                 bool zeroPad) {
  char digit[32] = {0};
  memset(digit, 0, 32);
  strcpy(digit, "0123456789");

  if (base == 16) {
    strcat(digit, "ABCDEF");
  } else if (base == 17) {
    strcat(digit, "abcdef");
    base = 16;
  }

  char *p = b;
  if (plusSignIfNeeded) {
    *p++ = '+';
  } else if (!plusSignIfNeeded && spaceSignIfNeeded) {
    *p++ = ' ';
  }

  uintmax_t shifter = i;
  do {
    ++p;
    shifter = shifter / base;
  } while (shifter);

  *p = '\0';
  do {
    *--p = digit[i % base];
    i = i / base;
  } while (i);

  int padding = paddingNo - (int)strlen(b);
  if (padding < 0)
    padding = 0;

  if (justify) {
    while (padding--) {
      if (zeroPad) {
        b[strlen(b)] = '0';
      } else {
        b[strlen(b)] = ' ';
      }
    }

  } else {
    char a[256] = {0};
    while (padding--) {
      if (zeroPad) {
        a[strlen(a)] = '0';
      } else {
        a[strlen(a)] = ' ';
      }
    }
    strcat(a, b);
    strcpy(b, a);
  }

  return b;
}

void displayCharacter(char c, int *a) {
  putchar(c);
  *a += 1;
}

void displayString(char *c, int *a) {
  for (int i = 0; c[i]; ++i) {
    displayCharacter(c[i], a);
  }
}

int vprintf(const char *format, va_list list) {
  LOCK(printf_lock);

  int chars = 0;
  char intStrBuffer[256] = {0};

  for (int i = 0; format[i]; ++i) {

    char specifier = '\0';
    char length = '\0';

    int lengthSpec = 0;
    int precSpec = 0;
    bool leftJustify = false;
    bool zeroPad = false;
    bool spaceNoSign = false;
    bool altForm = false;
    bool plusSign = false;
    int expo = 0;

    if (format[i] == '%') {
      ++i;

      bool extBreak = false;
      while (1) {

        switch (format[i]) {
          case '-':
            leftJustify = true;
            ++i;
            break;

          case '+':
            plusSign = true;
            ++i;
            break;

          case '#':
            altForm = true;
            ++i;
            break;

          case ' ':
            spaceNoSign = true;
            ++i;
            break;

          case '0':
            zeroPad = true;
            ++i;
            break;

          default:
            extBreak = true;
            break;
        }

        if (extBreak)
          break;
      }

      while (isdigit(format[i])) {
        lengthSpec *= 10;
        lengthSpec += format[i] - 48;
        ++i;
      }

      if (format[i] == '*') {
        lengthSpec = va_arg(list, int);
        ++i;
      }

      if (format[i] == '.') {
        ++i;
        while (isdigit(format[i])) {
          precSpec *= 10;
          precSpec += format[i] - 48;
          ++i;
        }

        if (format[i] == '*') {
          precSpec = va_arg(list, int);
          ++i;
        }
      } else {
        precSpec = 6;
      }

      if (format[i] == 'h' || format[i] == 'l' || format[i] == 'j' ||
          format[i] == 'z' || format[i] == 't' || format[i] == 'L') {
        length = format[i];
        ++i;
        if (format[i] == 'h') {
          length = 'H';
        } else if (format[i] == 'l') {
          length = 'q';
          ++i;
        }
      }
      specifier = format[i];

      memset(intStrBuffer, 0, 256);

      int base = 10;
      if (specifier == 'o') {
        base = 8;
        specifier = 'u';
        if (altForm) {
          displayString("0", &chars);
        }
      }
      if (specifier == 'p') {
        base = 16;
        length = 'z';
        specifier = 'u';
      }
      switch (specifier) {
        case 'X':
          base = 16;
        case 'x':
          base = base == 10 ? 17 : base;
          if (altForm) {
            displayString("0x", &chars);
          }

        case 'u': {
          switch (length) {
            case 0: {
              unsigned int integer = va_arg(list, unsigned int);
              __uint_str(integer, intStrBuffer, base, plusSign, spaceNoSign,
                         lengthSpec, leftJustify, zeroPad);
              displayString(intStrBuffer, &chars);
              break;
            }
            case 'H': {
              unsigned char integer = (unsigned char)va_arg(list, unsigned int);
              __uint_str(integer, intStrBuffer, base, plusSign, spaceNoSign,
                         lengthSpec, leftJustify, zeroPad);
              displayString(intStrBuffer, &chars);
              break;
            }
            case 'h': {
              unsigned short int integer = va_arg(list, unsigned int);
              __uint_str(integer, intStrBuffer, base, plusSign, spaceNoSign,
                         lengthSpec, leftJustify, zeroPad);
              displayString(intStrBuffer, &chars);
              break;
            }
            case 'l': {
              unsigned long integer = va_arg(list, unsigned long);
              __uint_str(integer, intStrBuffer, base, plusSign, spaceNoSign,
                         lengthSpec, leftJustify, zeroPad);
              displayString(intStrBuffer, &chars);
              break;
            }
            case 'q': {
              unsigned long long integer = va_arg(list, unsigned long long);
              __uint_str(integer, intStrBuffer, base, plusSign, spaceNoSign,
                         lengthSpec, leftJustify, zeroPad);
              displayString(intStrBuffer, &chars);
              break;
            }
            case 'j': {
              uintmax_t integer = va_arg(list, uintmax_t);
              __uint_str(integer, intStrBuffer, base, plusSign, spaceNoSign,
                         lengthSpec, leftJustify, zeroPad);
              displayString(intStrBuffer, &chars);
              break;
            }
            case 'z': {
              size_t integer = va_arg(list, size_t);
              __uint_str(integer, intStrBuffer, base, plusSign, spaceNoSign,
                         lengthSpec, leftJustify, zeroPad);
              displayString(intStrBuffer, &chars);
              break;
            }
            case 't': {
              ptrdiff_t integer = va_arg(list, ptrdiff_t);
              __uint_str(integer, intStrBuffer, base, plusSign, spaceNoSign,
                         lengthSpec, leftJustify, zeroPad);
              displayString(intStrBuffer, &chars);
              break;
            }
            default:
              break;
          }
          break;
        }

        case 'd':
        case 'i': {
          switch (length) {
            case 0: {
              int integer = va_arg(list, int);
              __int_str(integer, intStrBuffer, base, plusSign, spaceNoSign,
                        lengthSpec, leftJustify, zeroPad);
              displayString(intStrBuffer, &chars);
              break;
            }
            case 'H': {
              signed char integer = (signed char)va_arg(list, int);
              __int_str(integer, intStrBuffer, base, plusSign, spaceNoSign,
                        lengthSpec, leftJustify, zeroPad);
              displayString(intStrBuffer, &chars);
              break;
            }
            case 'h': {
              short int integer = va_arg(list, int);
              __int_str(integer, intStrBuffer, base, plusSign, spaceNoSign,
                        lengthSpec, leftJustify, zeroPad);
              displayString(intStrBuffer, &chars);
              break;
            }
            case 'l': {
              long integer = va_arg(list, long);
              __int_str(integer, intStrBuffer, base, plusSign, spaceNoSign,
                        lengthSpec, leftJustify, zeroPad);
              displayString(intStrBuffer, &chars);
              break;
            }
            case 'q': {
              long long integer = va_arg(list, long long);
              __int_str(integer, intStrBuffer, base, plusSign, spaceNoSign,
                        lengthSpec, leftJustify, zeroPad);
              displayString(intStrBuffer, &chars);
              break;
            }
            case 'j': {
              intmax_t integer = va_arg(list, intmax_t);
              __int_str(integer, intStrBuffer, base, plusSign, spaceNoSign,
                        lengthSpec, leftJustify, zeroPad);
              displayString(intStrBuffer, &chars);
              break;
            }
            case 'z': {
              size_t integer = va_arg(list, size_t);
              __int_str(integer, intStrBuffer, base, plusSign, spaceNoSign,
                        lengthSpec, leftJustify, zeroPad);
              displayString(intStrBuffer, &chars);
              break;
            }
            case 't': {
              ptrdiff_t integer = va_arg(list, ptrdiff_t);
              __int_str(integer, intStrBuffer, base, plusSign, spaceNoSign,
                        lengthSpec, leftJustify, zeroPad);
              displayString(intStrBuffer, &chars);
              break;
            }
            default:
              break;
          }
          break;
        }

        case 'c': {
          if (length == 'l') {
            displayCharacter(va_arg(list, int), &chars);
          } else {
            displayCharacter(va_arg(list, int), &chars);
          }

          break;
        }

        case 's': {
          displayString(va_arg(list, char *), &chars);
          break;
        }

        case 'n': {
          switch (length) {
            case 'H':
              *(va_arg(list, signed char *)) = chars;
              break;
            case 'h':
              *(va_arg(list, short int *)) = chars;
              break;

            case 0: {
              int *a = va_arg(list, int *);
              *a = chars;
              break;
            }

            case 'l':
              *(va_arg(list, long *)) = chars;
              break;
            case 'q':
              *(va_arg(list, long long *)) = chars;
              break;
            case 'j':
              *(va_arg(list, intmax_t *)) = chars;
              break;
            case 'z':
              *(va_arg(list, size_t *)) = chars;
              break;
            case 't':
              *(va_arg(list, ptrdiff_t *)) = chars;
              break;
            default:
              break;
          }
          break;
        }

        default:
          break;
      }

      if (specifier == 'e') {
        displayString("e+", &chars);
      } else if (specifier == 'E') {
        displayString("E+", &chars);
      }

      if (specifier == 'e' || specifier == 'E') {
        __int_str(expo, intStrBuffer, 10, false, false, 2, false, true);
        displayString(intStrBuffer, &chars);
      }

    } else {
      displayCharacter(format[i], &chars);
    }
  }

  UNLOCK(printf_lock);

  return chars;
}

__attribute__((format(printf, 1, 2))) int printf(const char *format, ...) {
  va_list list;
  va_start(list, format);
  int i = vprintf(format, list);
  va_end(list);
  return i;
}
