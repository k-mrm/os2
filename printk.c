#include <types.h>
#include <printk.h>
#include <console.h>
#include "string.h"

static uint
sprintiu32 (char *p, i32 num, int base, bool sign)
{
  char buf[sizeof (num) * 8 + 1] = {0};
  char *end = buf + sizeof (buf);
  char *cur = end - 1;
  bool neg = false;
  u32 unum;
  uint n = 0;

  if (sign && num < 0) {
    unum = (u32)(-(num + 1)) + 1;
    neg = true;
  } else {
    unum = (u32)num;
  }

  do {
    *--cur = "0123456789abcdef"[unum % base];
    n++;
  } while (unum /= base);

  if (neg) {
    *--cur = '-';
    n++;
  }

  memcpy (p, cur, n);
  return n;
}

static uint
sprintiu64 (char *p, i64 num, int base, bool sign)
{
  char buf[sizeof (num) * 8 + 1] = {0};
  char *end = buf + sizeof (buf);
  char *cur = end - 1;
  bool neg = false;
  u64 unum;
  uint n = 0;

  if (sign && num < 0) {
    unum = (u64)(-(num + 1)) + 1;
    neg = true;
  } else {
    unum = (u64)num;
  }

  do {
    *--cur = "0123456789abcdef"[unum % base];
    n++;
  } while (unum /= base);

  if (neg) {
    *--cur = '-';
    n++;
  }

  memcpy (p, cur, n);
  return n;
}

int
vsprintf (char *buf, const char *fmt, va_list ap)
{
  uint n = 0;
  uint len;
  char c;

  for (int i = 0; fmt[i]; i++) {
    c = fmt[i];
    if (c == '%') {
      c = fmt[++i];

      switch (c) {
        case 'd':
          len = sprintiu32 (buf + n, va_arg (ap, i32), 10, true);
          n += len;
          break;
        case 'u':
          len = sprintiu32 (buf + n, va_arg (ap, u32), 10, false);
          n += len;
          break;
        case 'x':
          len = sprintiu64 (buf + n, va_arg (ap, u64), 16, false);
          n += len;
          break;
        case 'p':
          buf[n++] = '0';
          buf[n++] = 'x';
          len = sprintiu64 (buf + n, va_arg (ap, u64), 16, false);
          n += len;
          break;
        case 'c': {
          int ch = va_arg (ap, int);
          buf[n++] = ch;
          break;
        }
        case 's': {
          char *s = va_arg (ap, char *);
          if (s == NULL)
            s = "(null)";

          len = strlen (s);
          memcpy (buf + n, s, len);
          n += len;
          break;
        }
        case '%':
          buf[n++] = '%';
          break;
        default:
          buf[n++] = '%';
          buf[n++] = c;
          break;
      }
    } else {
      buf[n++] = c;
    }
  }

  return n;
}

int
sprintf (char *buf, const char *fmt, ...)
{
  va_list ap;
  int n;

  va_start (ap, fmt);
  n = vsprintf (buf, fmt, ap);
  va_end (ap);
  return n;
}

int
printkcs (Console *cs, const char *fmt, va_list ap)
{
  char buf[512] = {0};
  int n;

  n = vsprintf (buf, fmt, ap);
  cs->write (cs, buf, n);
  return n;
}

int
printk (const char *fmt, ...)
{
  Console *cs = console;
  va_list ap;
  int n;

  if (!cs)
    return -1;

  va_start (ap, fmt);
  n = printkcs (cs, fmt, ap);
  va_end(ap);

  return n;
}
