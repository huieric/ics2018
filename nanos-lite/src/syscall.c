#include "common.h"
#include "syscall.h"
#include "proc.h"

void sys_yield(_Context* c);
void sys_exit(_Context* c, int code);
void sys_write(_Context* c);
void sys_brk(_Context* c);
void sys_open(_Context* c);
void sys_read(_Context* c);
void sys_close(_Context* c);
void sys_lseek(_Context* c);
void sys_execve(_Context* c);

int fs_open(const char* pathname, int flags, int mode);
size_t fs_read(int fd, void* buf, size_t len);
size_t fs_write(int fd, void* buf, size_t len);
int fs_close(int fd);
size_t fs_lseek(int fd, size_t offset, int whence);

void naive_uload(PCB* pcb, const char* filename);

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
    case SYS_execve: sys_execve(c); break;
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
  // naive_uload(NULL, "/bin/init");
  // assert(0);
}

void sys_open(_Context* c) {
  const char* path = (const char*)c->GPR2;
  int flags = c->GPR3;
  int mode = c->GPR4;
  c->GPR1 = fs_open(path, flags, mode);
}

void sys_read(_Context* c) {
  int fd = c->GPR2;
  void* buf = (void*)c->GPR3;
  size_t len = c->GPR4;
  c->GPR1 = fs_read(fd, buf, len);
}

void sys_write(_Context* c) {
  int fd = c->GPR2;
  void* buf = (void*)c->GPR3;
  size_t len = c->GPR4;
  if (fd == 1 || fd == 2 || fd == 6) {
    Log("fd=%d len=%u buf=%s", fd, len, buf);
  }
  c->GPR1 = fs_write(fd, buf, len);
}

void sys_close(_Context* c) {
  int fd = c->GPR2;
  c->GPR1 = fs_close(fd);
}

void sys_lseek(_Context* c) {
  int fd = c->GPR2;
  size_t offset = c->GPR3;
  int whence = c->GPR4;
  c->GPR1 = fs_lseek(fd, offset, whence);
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

void sys_execve(_Context* c) {
  const char *fname = (const char *)c->GPR2;
  // char * const *argv = (char * const *)c->GPR3;
  // char *const *envp = (char * const *)c->GPR4;
  Log("fname=%s", fname);
  naive_uload(NULL, fname);
  assert(0);
}