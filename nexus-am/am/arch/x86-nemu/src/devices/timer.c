#include <am.h>
#include <x86.h>
#include <amdev.h>

#define RTC_PORT 0x48

static uint32_t am_init_time;

size_t timer_read(uintptr_t reg, void *buf, size_t size) {
  uint32_t now = inl(RTC_PORT);
  switch (reg) {
    case _DEVREG_TIMER_UPTIME: {
      _UptimeReg *uptime = (_UptimeReg *)buf;
      *(uint32_t*)uptime = now - am_init_time;
      return sizeof(_UptimeReg);
    }
    case _DEVREG_TIMER_DATE: {
      _RTCReg *rtc = (_RTCReg *)buf;
      rtc->second = 0;
      rtc->minute = 0;
      rtc->hour   = 0;
      rtc->day    = 0;
      rtc->month  = 0;
      rtc->year   = 2018;
      return sizeof(_RTCReg);
    }
  }
  return 0;
}

void timer_init() {
  am_init_time = inl(RTC_PORT);
}
