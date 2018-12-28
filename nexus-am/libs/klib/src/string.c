#include "klib.h"

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

size_t strlen(const char *s) {
  int len = 0;
  const char* p = s;
  while (*p) {
    len++;
    p++;
  }
  return len;
}

char *strcpy(char* dst,const char* src) {
  return strncpy(dst, src, strlen(src));
}

char* strncpy(char* dst, const char* src, size_t n) {
  char* p_dst = dst;
  const char* p_src = src;
  for (size_t i = 0; i < n && *p_src; i++, p_src++, p_dst++) {
    *p_dst = *p_src;
  }
  *p_dst = 0;
  return dst;
}

char* strcat(char* dst, const char* src) {
  char* p_dst = dst + strlen(dst);
  strcpy(p_dst, src);
  return dst;
}

int strcmp(const char* s1, const char* s2) {
  int len_s1 = strlen(s1);
  int len_s2 = strlen(s2);
  return strncmp(s1, s2, len_s1 < len_s2 ? len_s1 : len_s2);
}

int strncmp(const char* s1, const char* s2, size_t n) {
  const char* p_s1 = s1;
  const char* p_s2 = s2;
  for (size_t i = 0; i < n && *p_s1 && *p_s2; i++, p_s1++, p_s2++) {
    if (*p_s1 != *p_s2) {
      return *p_s1 - *p_s2;
    }
  }
  return 0;
}

void* memset(void* v,int c,size_t n) {
  char* p = (char*)v;
  for (size_t i = 0; i < n; i++, p++) {
    *p = c;
  }
  return v;
}

void* memcpy(void* out, const void* in, size_t n) {
  char* p_out = (char*)out;
  const char* p_in = (const char*)in;
  for (size_t i = 0; i < n; i++, p_out++, p_in++) {
    *p_out = *p_in;
  }
  return out;
}

int memcmp(const void* s1, const void* s2, size_t n){
  const char* p_s1 = (const char*)s1;
  const char* p_s2 = (const char*)s2;
  for (size_t i = 0; i < n; i++, p_s1++, p_s2++) {
    if (*p_s1 != *p_s2) {
      return *p_s1 - *p_s2;
    }
  }
  return 0;
}

char* my_itoa(int d, char* str) {
  int sign = d < 0 ? 1 : 0;
  char buf[4096] = { 0 };
  char* p = buf;
  if (sign) {
    *p++ = '-';
  }
  //处理0的特殊情况
  if (d == 0) {
    *buf = '0';
  }
  while (d) {
    *p++ = (d % 10) + '0';
    d /= 10;
  }
  return strcpy_rev(str, buf);
}

char* strcpy_rev(char* dst, const char* src) {
  char* p_dst = dst;
  const char* p_src = src + strlen(src);
  for (size_t i = 0; i < strlen(src); i++, p_dst++) {
    p_src--;
    *p_dst = *p_src;
  }
  *p_dst = 0;
  return dst;
}

int atoi(const char* str) {
  int result = 0;
  int sign = 0;
  const char* p_str = str;
  if (*p_str == '-') {
    sign = 1;
    p_str++;
  }
  while (*p_str) {
    result = result * 10 + (*p_str - '0');
    p_str++;
  }
  return sign ? -result : result;
}

char* i2hex(int d, char* str) {
  char hex[9] = { 0 };
  for (int i = 0; i < 8; i++) {
    hex[i] = (d >> ((7 - i) * 4)) & 0xf;
    if (hex[i] < 10) {
      hex[i] += '0';
    }
    else {
      hex[i] += 'a' - 10;
    }
  }
  char* p_hex = hex;
  while (*p_hex == '0' && *(p_hex + 1)) p_hex++;
  return strcpy(str, p_hex);
}

char* double2a(double f, char* str) {
  char* p_str = str + strlen(my_itoa(f, str));
  *p_str++ = '.';
  double rem = f - (int)f;
  if (rem == 0) {
    *p_str++ = '0';
  }
  assert(0 < rem && rem < 1);
  while (rem) {
    rem *= 10;
    *p_str++ = (int)rem + '0';
    rem = rem - (int)rem;
    assert(0 < rem && rem < 1);
  }
  *p_str = 0;
  return str;
}

#endif
