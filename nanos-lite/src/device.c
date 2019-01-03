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
  Log("offset=%u len=%u", offset, len);
  assert(0 <= offset && offset + len <= 128);
  for (int i = 0; i < len; i++) {
    ((char*)buf)[i] = dispinfo[offset + i];
  }
  return len;
}

size_t fb_write(const void *buf, size_t offset, size_t len) {
  Log("offset=%u len=%u", offset, len);
  int x = offset % screen_width();
  int y = offset / screen_width();
  int w = len / (screen_width() - x);
  int h = len / (screen_height() - y);
  draw_rect((uint32_t*)buf, x, y, w, h);
  return len;
}

void init_device() {
  Log("Initializing devices...");
  _ioe_init();

  // TODO: print the string to array `dispinfo` with the format
  // described in the Navy-apps convention
  sprintf(dispinfo, "WIDTH:%d HEIGHT:%d", screen_width(), screen_height());
}
