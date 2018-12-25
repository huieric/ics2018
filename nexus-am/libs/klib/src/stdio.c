#include "klib.h"
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

int printf(const char *fmt, ...) {
  char buf[4096] = { 0 };
  va_list args;
  va_start(args, fmt);
  int n = vsprintf(buf, fmt, args);
  va_end(args);
  for (int i = 0; i < n; i++) {
    _putc(buf[i]);
  }
  return n;
}

int vsprintf(char *out, const char *fmt, va_list ap) {
  int d, len;
  char* s;
  char buf[4096] = { 0 };
  len = 0;
  while (*fmt) {
    if (*fmt == '%') {
      int n;
      switch (*(fmt + 1)) {
	case 's': s = va_arg(ap, char*);
		  n = sprintf(out, s);
		  break;
	case 'd': d = va_arg(ap, int);
		  n = sprintf(out, itoa(d, buf));
		  break;
	default: assert(0);
      }
      assert(n >= 0);
      fmt += 2;
      out += n;
      len += n;
    }
    else {
      *out++ = *fmt++;
      len++;
    }
  }
  return len;
}

int sprintf(char *out, const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  int n = vsprintf(out, fmt, args);
  va_end(args);
  return n;
}

int snprintf(char *out, size_t n, const char *fmt, ...) {
  char buf[4096] = { 0 };
  va_list args;
  va_start(args, fmt);
  int len = vsprintf(out, fmt, args);
  va_end(args);
  if (len < 0) {
    return len;
  }

  strncpy(out, buf, n);
  return len < n ? len : n;
}

#endif
