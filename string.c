#include "types.h"
#include "string.h"

void *
memcpy (void *dst, const void *src, ulong n)
{
        return memmove (dst, src, n);
}

void *
memmove (void *dst, const void *src, ulong n)
{
        char *d = dst;
        const char *s = src;

        if (s > d)
        {
                while (n-- > 0)
                        *d++ = *s++;
        }
        else
        {
                d += n;
                s += n;
                while (n-- > 0)
                        *--d = *--s;
        }
        return dst;
}

void *
memset (void *dst, int c, ulong n)
{
        char *d = dst;

        while (n-- > 0)
                *d++ = c;
        return dst;
}

char *
strcpy (char *dst, const char *src)
{
        char *r = dst;

        while ((*dst++ = *src++) != 0)
                ;
        return r;
}

int
strcmp (const char *s1, const char *s2)
{
        while (*s1 && *s1 == *s2)
        {
                s1++;
                s2++;
        }
        return *s1 - *s2;
}

int
strncmp (const char *s1, const char *s2, ulong len)
{
        while (*s1 && *s1 == *s2 && len > 0)
        {
                s1++;
                s2++;
                len--;
        }
        if (len == 0)
                return 0;
        return *s1 - *s2;
}

uint
strlen (const char *s)
{
        uint i = 0;
        while (*s++)
                i++;
        return i;
}
