#include "common.h"
#include <amdev.h>
#include "proc.h"

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
  int kc = read_key(), l;
  
  if ((kc & 0xfff) == _KEY_NONE) {
    uint32_t ut = uptime();
    l = sprintf(buf, "t %d\n", ut);
  } else {
    if (kc & 0x8000) {
      l = sprintf(buf, "kd %s\n", keyname[kc & 0xfff]);
      if ((kc & 0xfff) == _KEY_F1) {
        Log("F1: %p", fg_pcb);
        fg_pcb = pcbs[1];
        
      } else if ((kc & 0xfff) == _KEY_F2) {
        
        fg_pcb = pcbs[2];
        Log("F2: %p", fg_pcb);
      }
      Log("events_read: %s", buf);
    } else {
      l = sprintf(buf, "ku %s\n", keyname[kc & 0xfff]);
      Log("events_read: %s", buf);
    }
  }
  return l;
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
  sprintf(dispinfo, "WIDTH:%d\nHEIGHT:%d\n", screen_width(), screen_height());
}
