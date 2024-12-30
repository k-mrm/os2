#ifndef _COMPILER_H
#define _COMPILER_H

#define UNUSED          __attribute__ ((unused))
#define DEBUG           __attribute__ ((unused))
#define FALLTHROUGH     __attribute__ ((fallthrough))
#define PACKED          __attribute__ ((packed))
#define ALIGNED(n)      __attribute__ ((aligned(n)))
#define NORETURN        __attribute__ ((noreturn))
#define SECTION(s)      __attribute__ ((section(s)))
#define USED            __attribute__ ((used))

#define LIKELY(cond)    __builtin_expect (!!(cond), 1)
#define UNLIKELY(cond)  __builtin_expect (!!(cond), 0)

#define INIT            SECTION (".text.init")

#define va_list         __builtin_va_list
#define va_start(v, l)  __builtin_va_start (v, l)
#define va_arg(v, l)    __builtin_va_arg (v, l)
#define va_end(v)       __builtin_va_end (v)
#define va_copy(d, s)   __builtin_va_copy (d, s)

#endif  /* _COMPILER_H */
