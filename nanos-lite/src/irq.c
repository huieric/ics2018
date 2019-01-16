#include "common.h"

_Context* do_syscall(_Context* c);
_Context* schedule(_Context *prev);

static _Context* do_event(_Event e, _Context* c) {
  _Context* cp = NULL;
  switch (e.event) {
    case _EVENT_IRQ_TIMER: Log("[%s]: receive _EVENT_IRQ_TIMER event", __func__); _yield(); break;
    case _EVENT_YIELD: cp = schedule(c); break;
    case _EVENT_SYSCALL: do_syscall(c); break;
    default: panic("Unhandled event ID = %d", e.event);
  }

  return cp;
}

void init_irq(void) {
  Log("Initializing interrupt/exception handler...");
  _cte_init(do_event);
}
