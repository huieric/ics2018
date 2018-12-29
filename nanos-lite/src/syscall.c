#include "common.h"
#include "syscall.h"

void sys_yield(_Context* c);
void sys_exit(_Context* c, int code);
void sys_write(_Context* c);

_Context* do_syscall(_Context *c) {
  uintptr_t a[4];
  a[0] = c->GPR1;
  
  switch (a[0]) {
    case SYS_exit: sys_exit(c, 0); break;
    case SYS_yield: sys_yield(c); break;
    case SYS_write: sys_write(c); break;
    default: panic("Unhandled syscall ID = %d", a[0]);
  }

  return NULL;
}

void sys_yield(_Context* c) {
  _yield();
  c->GPR1 = 0;
}

void sys_exit(_Context* c, int code) {
  _halt(code);
  c->GPR1 = 0;
}

void sys_write(_Context* c) {
  char* buf = (char*)c->GPR3;
  size_t len = c->GPR4;
  switch (c->GPR2) {
    case 1: for (size_t i = 0; i < len; i++) {
	      _putc(buf[i]);
	    }
	    c->GPR1 = 0;
	    break;
    case 2: assert(0);
	    break;
    default: assert(0);
	     break;
  }
}
