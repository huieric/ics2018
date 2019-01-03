#include "common.h"
#include "syscall.h"

void sys_yield(_Context* c);
void sys_exit(_Context* c, int code);
void sys_write(_Context* c);
void sys_brk(_Context* c);

_Context* do_syscall(_Context *c) {
  uintptr_t a[4];
  a[0] = c->GPR1;
  switch (a[0]) {
    case SYS_exit: sys_exit(c, 0); break;
    case SYS_yield: sys_yield(c); break;
    case SYS_write: sys_write(c); break;
    case SYS_brk: sys_brk(c); break;
    case SYS_open: sys_open(c); break;
    case SYS_read: sys_read(c); break;
    case SYS_close: sys_close(c); break;
    case SYS_lseek: sys_lseek(c); break;
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
  int fd = c->GPR2;
  void* buf = (void*)c->GPR3;
  size_t len = c->GPR4;
  size_t i;
  switch (fd) {
    case 0: assert(0);
    case 1: char c = *(char*)(buf + i);
	    for (i = 0; i < len && c; i++) {
	      _putc(c);
	    }
	    c->GPR1 = i < len ? i : len;
	    break;
    case 2: assert(0);
	    break;
    defaulc->GPt: fs_write(fd, buf, len);
	     break;
  }
}

void sys_brk(_Context* c) {
  /*intptr_t addr = c->GPR2;*/
  c->GPR1 = 0;
  /*extern PCB* current;*/
  /*if (addr < current->max_brk) {*/
    /*current->cur_brk = addr;*/
    /*c->GPR1 = 0;*/
  /*}*/
  /*else {*/
    /*c->GPR1 = -1;*/
  /*}*/
}
