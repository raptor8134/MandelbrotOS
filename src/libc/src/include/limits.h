#define CHAR_BIT 8           // 8
#define SCHAR_MIN -127       // -(2^7-1)
#define SCHAR_MAX 127        // 2^7-1
#define UCHAR_MAX 256        // 2^8-1
#define CHAR_MIN SCHAR_MIN   // Machine dependent, but mostly signed on x86
#define CHAR_MAX SCHAR_MAX   // ^
#define MB_LEN_MAX 1         // No support for multibyte yet :(
#define SHRT_MIN -32767      // -(2^15-1)
#define SHRT_MAX 32767       // 2^15-1
#define USHRT_MAX 65535      // 2^16-1
#define INT_MIN -32767       // -(2^15-1)
#define INT_MAX 32767        // 2^15-1
#define UINT_MAX 65535       // 2^16-1
#define LONG_MIN -2147483647 // -(2^31-1)
#define LONG_MAX 2147483647  // 2^31-1
#define ULONG_MAX 4294967295 // 2^32-1
#define LLONG_MIN -9223372036854775807  // -(2^63-1)
#define LLONG_MAX 9223372036854775807   // 2^63-1
#define ULLONG_MAX 18446744073709551615 // 2^64-1
