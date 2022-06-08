#include <math.h>

/*
 * TODO move all these sections to seperate files
 *
 * Thanks to nvidia for doing one good thing for the world and 
 * having all these functions available for reference
 * https://developer.download.nvidia.com/cg/index_stdlib.html
 */

/* INVERSE TRIG */

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

/* HYPERBOLIC INVERSE TRIG */

/* HYPERBOLIC TRIG */

/* EXPONENTIALS */

/* LOGARITHMS */

/* POWER AND ABS */

/* ERROR AND GAMMA */

/* NEAREST INTEGER */

/* REMAINDER */

/* MANIPULATION */

/* DIFF MAX MIN */

/* FLOATING MULTIPLY ADD */
// Inlined because its short (2 and 1 instructions) in either case
// Note that it needs -O1 or greater and vfmadd213sd support
// (#define SUPPORTS_FMA_ASM ) for the 1 instruction scenario
inline double fma(double x, double y, double z){
    double retval;
#ifdef SUPPORTS_FMA_ASM 
    asm("vfmadd213sd %0, %1, %2" : "+x"(x) : "x"(y), "x"(z) );
    retval = x;
#else
    retval = x * y + z;
#endif
    return retval;
}

/* COMPARISON */
