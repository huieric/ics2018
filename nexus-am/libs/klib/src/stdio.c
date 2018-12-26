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
  memset(out, 0, sizeof(out));
  int len;
  char* s;
  char buf[4096] = { 0 };
  len = 0;
  while (*fmt) {
    if (*fmt == '%') {
      fmt++;
      int zero_flag = 0;
      if (*fmt == '0') {
	zero_flag = 1;
	fmt++;
      }
      const char* p_fmt = fmt;
      while (*p_fmt != 's' && *p_fmt != 'd') {
	p_fmt++;
      }
      char num[32] = { 0 };
      strncpy(num, fmt, p_fmt - fmt);
      fmt = p_fmt;

      int n;
      int size = atoi(num);
      switch (*fmt) {
	case 's': s = va_arg(ap, char*);
    		  break;
	case 'd': s = itoa(va_arg(ap, int), buf);
		  break;
	default: assert(0);
      }

      size = size ? size : strlen(s);
      assert(size >= strlen(s));
      n = strlen(strcat(memset(out, zero_flag ? '0' : ' ', size - strlen(s)), s));

      assert(n >= 0);
      fmt++;
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
