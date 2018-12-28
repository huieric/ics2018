#include "common.h"
#include "syscall.h"

void sys_yield(_Context* c);
void sys_exit(_Context* c);

_Context* do_syscall(_Context *c) {
  uintptr_t a[4];
  a[0] = c->GPR1;
  
  switch (a[0]) {
    case 0: sys_exit(c); break;
    case 1: sys_yield(c); break;
    default: panic("Unhandled syscall ID = %d", a[0]);
  }

  return NULL;
}

void sys_yield(_Context* c) {
  _yield();
  c->GPR1 = 0;
}

void sys_exit(_Context* c) {
  Log("Good");
  _halt(c->GPR2);
  c->GPR1 = 0;
}
