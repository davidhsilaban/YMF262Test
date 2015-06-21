#include <string.h>
#if 1
#define logerror(...) fprintf (stderr, __VA_ARGS__)
#else
#define logerror(...) {                         \
__asm__ ("int3");                               \
fprintf (stderr, "YMF262: "__VA_ARGS__);        \
} while (0)
#endif
#define INLINE __inline
#ifndef HAS_YMF262
#define HAS_YMF262 1
#endif
#ifndef QEMU
#define QEMU
#endif
