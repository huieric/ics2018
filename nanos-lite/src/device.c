#include "common.h"
#include <amdev.h>

size_t serial_write(const void *buf, size_t offset, size_t len) {
  for (int i = 0; i < len; i++) {
    _putc(((const char*)buf)[i]);
  }
  return len;  
}

#define NAME(key) \
  [_KEY_##key] = #key,

static const char *keyname[256] __attribute__((used)) = {
  [_KEY_NONE] = "NONE",
  _KEYS(NAME)
};

size_t events_read(void *buf, size_t offset, size_t len) {
  
  return 0;
}

static char dispinfo[128] __attribute__((used));

size_t dispinfo_read(void *buf, size_t offset, size_t len) {
  if (offset > strlen(dispinfo)) {
    return 0;
  }
  if (offset + len > strlen(dispinfo)) {
    len = strlen(dispinfo) - offset;
  }
  for (int i = 0; i < len; i++) {
    ((char*)buf)[i] = dispinfo[offset + i];
  }
  return len;
}

size_t fb_write(const void *buf, size_t offset, size_t len) {
  int x = (offset / sizeof(uint32_t)) % screen_width();
  int y = (offset / sizeof(uint32_t)) / screen_width();
  int w = len / sizeof(uint32_t);
  int h = 1;
  draw_rect((uint32_t*)buf, x, y, w, h);
  return len;
}

void init_device() {
  Log("Initializing devices...");
  _ioe_init();

  // TODO: print the string to array `dispinfo` with the format
  // described in the Navy-apps convention
  sprintf(dispinfo, "WIDTH:%d\nHEIGHT:%d\n", screen_width(), screen_height());
  Log("width=%d height=%d", screen_width(), screen_height());
}
