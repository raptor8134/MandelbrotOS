#include <math.h>

/*
 * TODO move all these sections to seperate files
 * TODO dont use so much asm
 * Note that `#define`s are in this file commented out, so that you don't
 * have to switch between this and <math.h> to see what a macro does
 *
 */

/* INVERSE TRIG */

/* TRIG */
// TODO change to a more efficient algorithm, the asm instruction
// sucks but will work for now
// Helpful resources for asm and constraints:
// https://www.felixcloutier.com/x86/
// https://www.linuxtopia.org/online_books/programming_tool_guides/linux_using_gnu_compiler_collection/constraints.html
double cos(double x) {
  double retval;
  asm("fcos" : "=t"(retval) : "0"(x));
  return retval;
}

double sin(double x) {
  double retval;
  asm("fsin" : "=t"(retval) : "0"(x));
  return retval;
}

double
tan(double x) { // using fsincos since its faster than sequential fsin and fcos
  double s, c;
  asm("fsincos" : "=t"(s), "=u"(c) : "0"(x));
  return s / c; // TODO handle floating point exceptions TODO
}

/* HYPERBOLIC INVERSE TRIG */

/* HYPERBOLIC TRIG */

/* EXPONENTIALS */

/* LOGARITHMS */
double log2(double x) {
  double retval;
  asm("fyl2x" : "=t"(retval) : "0"(x), "u"(1.0));
  return retval;
}

/* POWER AND ABS */
// In header file
//#define fabs(x) (x > 0 ? x : -x)
//#define fabsf fabs
//#define fabsl fabs

double hypot(double x, double y) { return sqrt(x * x + y * y); }
float hypotf(float x, float y) { return (float)sqrt(x * x + y * y); }

double sqrt(double x) {
  double retval;
  asm("sqrtsd %0, %1" : "=x"(retval) : "x"(x));
  return retval;
}
float sqrtf(float x) { return (float)sqrt(x); }

/* ERROR AND GAMMA */

/* NEAREST INTEGER */

/* REMAINDER */

/* MANIPULATION */

/* DIFF MAX MIN */
double fdim(double x, double y) { return fabs(x - y); }
float fdimf(float x, float y) { return fabsf(x - y); }
long double fdiml(long double x, long double y) { return fabsl(x - y); }

// In header file
//#define fmin(x, y) (x > y ? y : x)
//#define fminf(x, y) (x > y ? y : x)
//#define fminl(x, y) (x > y ? y : x)

// In header file
//#define fmax(x, y) (x > y ? x : y)
//#define fmaxf(x, y) (x > y ? x : y)
//#define fmaxl(x, y) (x > y ? x : y)

/* FLOATING MULTIPLY ADD */
// Inlined because its short (2 and 1 instructions) in either case
// Note that it needs -O1 or greater and vfmadd213sd support
// (#define SUPPORTS_FMA_ASM ) for the 1 instruction scenario
inline double fma(double x, double y, double z) {
  double retval;
#ifdef SUPPORTS_FMA_ASM
  asm("vfmadd213sd %0, %1, %2" : "+x"(x) : "x"(y), "x"(z));
  retval = x;
#else
  retval = x * y + z;
#endif
  return retval;
}

/* COMPARISON */
