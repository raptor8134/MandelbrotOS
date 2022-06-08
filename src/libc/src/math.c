#include <math.h>

/* TRIG */
// TODO change to a more efficient algorithm, the asm instruction
// sucks but will work for now
double cos(double x) {
  double retval;
  asm ("fcos" : "=t"(retval) : "0"(x));
  return retval;
}

double sin(double x) {
  double retval;
  asm ("fsin" : "=t"(retval) : "0"(x));
  return retval;
}

double tan(double x) { // using fsincos since its faster than sequential fsin and fcos
  double s, c;
  asm("fsincos" : "=t"(s), "=u"(c) : "0"(x));
  return s/c;
}
