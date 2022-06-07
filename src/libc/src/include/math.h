#ifndef __MATH_H__
#define __MATH_H__

/*
 * Trying to make the math library up to C99 standard
 * Reference the section '7.12 Mathematics <math.h>' in
 * https://www.open-std.org/jtc1/sc22/wg14/www/docs/n1256.pdf
 * (C99 standard pdf) for all the required functions
 *
 * See other big block comment for more stuff.
 */

// TODO Implement `FP_CONTRACT` pragma

/* CONSTANTS copied from glibc <math.h> header because lazy */
#define M_E 2.7182818284590452354         /* e          */
#define M_LOG2E 1.4426950408889634074     /* log_2 e    */
#define M_LOG10E 0.43429448190325182765   /* log_10 e   */
#define M_LN2 0.69314718055994530942      /* log_e 2    */
#define M_LN10 2.30258509299404568402     /* log_e 10   */
#define M_PI 3.14159265358979323846       /* pi         */
#define M_PI_2 1.57079632679489661923     /* pi/2       */
#define M_PI_4 0.78539816339744830962     /* pi/4       */
#define M_1_PI 0.31830988618379067154     /* 1/pi       */
#define M_2_PI 0.63661977236758134308     /* 2/pi       */
#define M_2_SQRTPI 1.12837916709551257390 /* 2/sqrt(pi) */
#define M_SQRT2 1.41421356237309504880    /* sqrt(2)    */
#define M_SQRT1_2 0.70710678118654752440  /* 1/sqrt(2)  */

/**************/

/* MACROS */
// TODO make a <limits.h> so this is more sane
#define float_t float
#define double_t double
#define FLT_EVAL_METHOD 0
#define HUGE_VAL 0x7FF0000000000000 // 64 bit IEEE 754 infinity
#define HUGE_VALF INFINITY
#if SIZEOF_LONG_DOUBLE > SIZEOF_DOUBLE
#define HUGE_VALL 0x7FFF8000000000000000 // 80 bit IEEE 754 infinity
#else
#define HUGE_VALL HUGE_VAL
#endif
#define INFINITY 0x7F800000
#define NAN 0x7FC00000 // can be many others
#define FP_NAN 0       // gcc defines these five this way so I am too
#define FP_INFINITE 1
#define FP_ZERO 2
#define FP_SUBNORMAL 3
#define FP_NORMAL 4
//#define FP_FAST_FMA // not sure if we need these three or not yet
//#define FP_FAST_FMAF
//#define FP_FAST_FMAL
#define FP_ILOGB0 2147483647   // TODO redefine as INT_MAX when have <limits.h>
#define FP_ILOGBNAN 2147483647 // same as above
#define MATH_ERRNO 1
#define MATH_ERREXCEPT 2
#define math_errhandling 3

/**************/

/* CLASSIFICATION MACROS */
//#define fpclassify(x) __fpclassify(x) // this function will be in math.c
#define isfinite(x) //
#define isinf(x) (x == INFINITY || x == -INFINITY)
#define isnan(x) (x != x) // NaN is the only one not equal to itself
//#define isnormal(x) // Don't know how to implement yet
#define signbit(x) (x < 0) // fix for signed zeros/NaNs, good enough for now tho

/**************/

/*
 * TODO Uncomment lines when implemented in math.c
 * Only do the normal ones for now, save floats and ldubs for later.
 *
 * Note that these are currently ordered how they are in the standard,
 * but that doesn't really matter just put the most important at the top
 * if you want to. Or maybe it does matter who knows.
 *
 * Complex functions will be in <complex.h>, and integer arithmetic mostly goes
 * in <stdlib.h> so look there if you don't find something here
 *
 * One more thing: those of these that are implemented will be in `math.c` for a
 * while, but will move to seperate files based on category in the directory
 * libc/math/ sometime in the future.
 */

/* INVERSE TRIG */
// double acos(double x);
// float acosf(double x);
// long double acos21(long double x);

// double asin(double x);
// float asinf(double x);
// long double asin21(long double x);

// double atan(double x);
// float atanf(double x);
// long double atan1(long double x);

// double atan2(double y, long double x);
// float atan2(double y, long double x);
// long double atan21(long double y, long double x);

/**************/

/* TRIG */
// double cos(double x);
// float cosf(double x);
// long double cos1(long double x);

// double sin(double x);
// float sinf(double x);
// long double sin(long double x);

// double tan(double x);
// float tanf(double x);
// long double tan(long double x);

/**************/

/* HYPERBOLIC INVERSE TRIG */
// double acosh(double x);
// float acoshf(float x);
// long double acoshl(long double x);

// double asinh(double x);
// float asinhf(float x);
// long double asinhl(long double x);

// double atanh(double x);
// float atanhf(float x);
// long double atanhl(long double x);

/**************/

/* HYPERBOLIC TRIG */
// double cosh(double x);
// float coshf(float x);
// long double coshl(long double x);

// double sinh(double x);
// float sinhf(float x);
// long double sinhl(long double x);

// double tanh(double x);
// float tanhf(float x);
// long double tanhl(long double x);

/**************/

/* EXPONENTIALS */
// double exp(double x);
// float expf(float x);
// long double expl(long double x);

// double exp2(double x);
// float exp2f(float x);
// long double exp2l(long double x);

// double expm1(double x);
// float expm1f(float x);
// long double expm1l(long double x);

// double frexp(double x);
// float frexpf(float x);
// long double frexpl(long double x);

/**************/

/* LOGARITHMS */
// int ilogb(double x);
// int ilogbf(float x);
// int ilogbl(long double x);

// double ldexp(double x, int exp);
// float ldexpf(float x, int exp);
// long double ldexpl(long double x, int exp);

// double log(double x);
// float logf(float x);
// long double logl(long double x);

// double log10(double x);
// float log10f(float x);
// long double log10l(long double x);

// double log1p(double x);
// float log1pf(float x);
// long double log1pl(long double x);

// double log2(double x);
// float log2f(float x);
// long double log2l(long double x);

// double logb(double x);
// float logbf(float x);
// long double logbl(long double x);

// double scalbn(double x, int n);
// float scalbnf(float x, int n);
// long double scalbnl(long double x, int n);

// double scalbln(double x, long int n);
// float scalblnf(float x, long int n);
// long double scalblnl(long double x, long int n);

/**************/

/* POWER AND ABS */
// double cbrt(double x);
// float cbrtf(float x);
// long double cbrtl(long double x);

// double fabs(double x);
// float fabsf(float x);
// long double fabsl(long double x);

// double hypot(double x, double y);
// float hypotf(float x, float y);
// long double hypotl(long double x, long double y);

// double pow(double x, double y);
// float powf(float x, float y);
// long double powl(long double x, long double y);

// double sqrt(double x);
// float sqrtf(float x);
// long double sqrtl(long double x);

/**************/

/* ERROR AND GAMMA */
// double erf(double x);
// float erff(float x);
// long double erfl(long double x);

// double erfc(double x){ return 1 - erf(x); }
// float erfcf(float x){ return 1 - erff(x); }
// long double erfcl(long double x){ return 1 - erfl(x); }

/* gamma functions */
// double lgamma(double x);
// float lgammaf(float x);
// long double lgammal(long double x);

// double tgamma(double x);
// float tgammaf(float x);
// long double tgammal(long double x);

/**************/

/* NEAREST INTEGER */
// double ceil(double x);
// float ceilf(float x);
// long double ceill(long double x);

// double floor(double x);
// float floorf(float x);
// long double floorl(long double x);

// double nearbyint(double x);
// float nearbyintf(float x);
// long double nearbyintl(long double x);

// double rint(double x);
// float rintf(float x);
// long double rintl(long double x);

// double round(double x);
// float roundf(float x);
// long double roundl(long double x);

// lont int lround(double x);
// lont int lroundf(float x);
// lont int lroundl(long double x);

// long long int llround(double x);
// long long int llroundf(float x);
// long long int llroundl(long double x);

// double trunc(double x);
// float truncf(float x);
// long double truncl(long double x);

/**************/

/* REMAINDER */
// double fmod(double x, double y);
// float fmodf(float x, float y);
// long double fmodl(long double x, long double y);

// double remainder(double x, double y);
// float remainderf(float x, float y);
// long double remainderl(long double x, long double y);

// double remquo(double x, double y, int *quo);
// float remquof(float x, float y, int *quo);
// long double remquol(long double x, long double y, int *quo);

/**************/

/* MANIPULATION */
// double copysign(double x, double y);
// float copysignf(float x, float y);
// long double copysignl(long double x, long double y);

// double nan(const char *tagp);
// float nanf(const char *tagp);
// long double nanl(const char *tagp);

// double nextafter(double x, double y);
// float nextafterf(float x, float y);
// long double nextafterl(long double x, long double y);

// double nexttoward(double x, long double y);
// float nexttowardf(float x, long double y);
// long double nexttowardl(long double x, long double y);

/**************/

/* DIFF MAX MIN */
// double fdim(double x, double y):
// float fdimf(float x, float y):
// long double fdiml(long double x, long double y):

// fmax(double x, double y);
// fmaxf(float x, float y);
// fmaxl(long double x, long double y);

// fmin(double x, double y);
// fminf(float x, float y);
// fminl(long double x, long double y);

/**************/

/* FLOATING MULTIPLY ADD */
// double fma(double x, double y, double z);
// float fmaf(float x, float y, float z);
// long double fmal(long double x, long double y, long double z);

/**************/

/* COMPARISON */
// TODO: implement NAN and whatever so these can work as intended
//#define isgreater(x,y)
//#define isgreaterequal(x,y)
//#define isless(x,y)
//#define islessequal(x,y)
//#define islessgreater(x,y)
//#define isunordered(x,y)

#endif
