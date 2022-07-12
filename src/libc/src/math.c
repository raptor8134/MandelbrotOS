#include <limits.h>
#include <math.h>
#include <stdint.h>
#include <stdlib.h>

/*
 * TODO move all these sections to seperate files
 * TODO dont use so much asm
 * TODO implement <errno.h> error handling for functions that need it
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
// TODO comment this more, the math is really cool
double exp(double x) {
  if (x < 0) { return 1/exp(-x); }
  double a = 0;
  // Taylor series
  long int center = (long int)round(x);
  long double cval = intpow(M_E, center);
  for (int n=31; n>=0; n--) {
    a += cval/factorial(n) * intpow(x-center, n);
  }
  // Newton's method
  for (int i=0; i<20; i++) {
    a = a * (1 + x - log(a));
  }
  return a;
}

// this one just uses a close to the origin Taylor series + Newton's,
// since its intended purpose is for small values of x
double expm1(double x) {
  double a = 0; 
  for (int n=1; n<5; n++) {
    a += intpow(x,n) / factorial(n);
  }
  for (int i=0; i<10; i++) {
    a = a * (1 + x - log(a));
  }
  return a;
}

/* LOGARITHMS */
int ilogb(double x) {
  int retval;
  if (x == 0) {
    retval = FP_ILOGB0;
  } else if (isinf(x)) {
    retval = INT_MAX;
  } else if (isnan(x)) {
    retval = FP_ILOGBNAN;
  } else {
    retval = (int)logb(x);
  }
  return retval;
}
int ilogbf(float x) { return ilogbf(x); }

// TODO actually make this optimal and not a disgusting mess that
// probably doesn't work
double ldexp(double x, int exp) {
  if (x == 0 || abs(x) == INFINITY || exp == 0 || isnan(x)) {
    return x;
  } else if (abs(exp) >= (int)sizeof(long long int)) {
    return x > 0 ? HUGE_VAL : -HUGE_VAL;
  } else if (x > 0) {
    return x * (1LL << exp);
  } else {
    return x / (1LL << (-exp));
  }
}

// https://cs.stackexchange.com/questions/91185/compute-ex-given-starting-approximation
double log(double x) {
  int m = 16;
  double s = x * (1<<m);
  return M_PI/(2*agm(1, 4/s)) - m*M_LN2;
}
float logf(float x) {
  return x;
}

double log10(double x) {
  // more log property magic, 1/log_2(10) = ln(2)/ln(10)
  return __fyl2x(x, M_LN2/M_LN10);
}
float log10f(float x) { return (float)__fyl2x((double)x, M_LN2/M_LN10); }

double log1p(double x) { return __fyl2x(x + 1.0, M_LOG2E); }
float log1pf(float x) { return (float)__fyl2x((double)x + 1.0, M_LOG2E); }

double log2(double x) { return __fyl2x(x, 1.0); }
float log2f(float x) { return (float)__fyl2x((double)x, 1.0); }

// base of logb is only not 2 on the IBM 360 and derivatives(ancient),
// so we can just use log2 for all cases
// if anyone gets this os to run on a 360 they can figure it out
double logb(double x) { return log2(fabs(x)); }
float logbf(float x) { return log2f(fabsf(x)); }

/* POWER AND ABS */
// In header file
//#define fabs(x) (x > 0 ? x : -x)
//#define fabsf(x) fabs(x)
//#define fabsl(x) fabs(x)

double hypot(double x, double y) { return sqrt(x * x + y * y); }
float hypotf(float x, float y) { return (float)sqrt(x * x + y * y); }

double pow(double x, double y) { return exp(y * log(x)); }

// https://math.stackexchange.com/questions/296102/fastest-square-root-algorithm
// https://en.wikipedia.org/wiki/Methods_of_computing_square_roots
// TODO figure out the optimal number of iterations for Newton's method 
// for any given argument
double sqrt(double x) {
  if (x < 0) { return NAN;}
  double a = 10 - 190/(x+20);
  for (int i=0; i<30; i++) {
    a = a - (a*a - x)/(2*a);
  }
  return a;
}
float sqrtf(float x) { return (float)sqrt(x); }

/* ERROR AND GAMMA */

/* NEAREST INTEGER */
double ceil(double x) { return (double)((long long int)x) + !__signbit(x); }
float ceilf(float x) { return (float)((long long int)x) + !__signbitf(x); }
long double ceill(long double x) {
  return (long double)((long long int)x) + !__signbitl(x);
}

double floor(double x) { return (double)((long long int)x + __signbit(x)); }
float floorf(float x) { return (float)((long long int)x) + __signbitf(x); }
long double floorl(long double x) {
  return (long double)((long long int)x) + __signbitl(x);
}

// TODO make round() branchless (something with signbit internals and xor
// prolly)
double round(double x) { return (long long int)(x + 0.5 * (x > 0 ? 1 : -1)); }
float roundf(float x) { return (long long int)floorf(x + 0.5 * (x > 0 ? 1 : -1)); }
long double roundl(long double x) { 
  return (long long int)floorl(x + 0.5 * (x > 0 ? 1 : -1));
}

double trunc(double x) { return (double)((long long int)x); }
float truncf(float x) { return (float)((long long int)x); }
long double truncl(long double x) { return (long double)((long long int)x); }

/* REMAINDER */
double fmod(double x, double y) {
  return x - (int)(x / y) * y; // TODO make this NaN and inf proof
}

double remainder(double x, double y) { return x - round(x / y) * y; }

// note that the standardese specifying modulo whatever for
// quo is just saying that integer overflows are ok here
double remquo(double x, double y, int *quo) {
  *quo = (int)(x / y);
  return remainder(x, y);
}

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

/* EXTRAS */
// factorial and intpow are basically internals for exp, but
// they might be useful to someone else
long long int factorial(int n) {
  int retval = 1;
  if (n > 1) {
    for (; n>0; n--) {
      retval *= n;
    }
  }
  return retval;
}

long double intpow(double x, long int y) {
  if (y < 0) { return intpow(x, -y); }
  long double retval = 1.0l;
  for (int i=0; i<y; i++) {
    retval *= x;
  }
  return retval;
}

// arithmetic-geometric mean, for natural log function
double agm(double x, double y) { 
    double a[2] = {x, 0}, g[2] = {y, 0};
    for (int i=0; i<10; i++) {
        a[1] = (a[0] + g[0])/2;
        g[1] = sqrt(a[0] * g[0]);
        a[0] = a[1]; g[0] = g[1];
    }
    return a[0];
}

/* INTERNALS */
// expose fyl2x asm instruction
// could be useful to you if you have to do a log in an arbitrary base
inline double __fyl2x(double x, double y) {
  double retval;
  asm("fyl2x" : "=t"(retval) : "0"(x), "u"(y));
  return retval;
}

// signbit internals
// TODO fix for possible big-endian scenario (<endian.h>?)
int __signbit(double x) {
  union {
    double d;
    uint8_t i[8];
  } u;
  u.d = x;
  return (u.i[7] & 0x80) >> 7;
}
int __signbitf(float x) {
  union {
    float d;
    uint8_t i[4];
  } u;
  u.d = x;
  return (u.i[3] & 0x80) >> 7;
}
int __signbitl(long double x) {
  union {
    long double d;
    uint8_t i[10];
  } u;
  u.d = x;
  return (u.i[9] & 0x80) >> 7;
}
