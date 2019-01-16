#include <am.h>
#include <x86.h>
#include <amdev.h>
#include <klib.h>

#define SCREEN_PORT 0x100

static uint32_t *const fb __attribute__((used)) = (uint32_t *)0x40000;

size_t video_read(uintptr_t reg, void *buf, size_t size)
{
  uint32_t video_info = inl(SCREEN_PORT);
  switch (reg)
  {
  case _DEVREG_VIDEO_INFO:
  {
    _VideoInfoReg *info = (_VideoInfoReg *)buf;
    info->width = (video_info >> 16) & 0xffff;
    info->height = video_info & 0xffff;
    return sizeof(_VideoInfoReg);
  }
  }
  return 0;
}

size_t video_write(uintptr_t reg, void *buf, size_t size)
{
  switch (reg)
  {
  case _DEVREG_VIDEO_FBCTL:
  {
    _FBCtlReg *ctl = (_FBCtlReg *)buf;
    int width = screen_width();
    for (int i = 0; i < ctl->h; i++)
    {
      memcpy(fb + (ctl->y + i) * width + ctl->x, ctl->pixels + i * ctl->w, ctl->w * 4);
    }

    if (ctl->sync)
    {
      // do nothing, hardware syncs.
    }
    return sizeof(_FBCtlReg);
  }
  }
  return 0;
}

void vga_init()
{
}
