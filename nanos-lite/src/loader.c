#include "proc.h"

#define DEFAULT_ENTRY 0x4000000

size_t ramdisk_read(void* buf, size_t offset, size_t size);
size_t get_ramdisk_size();
int fs_open(const char* pathname, int flags, int mode);
size_t fs_read(int fd, void* buf, size_t len);
int fs_close(int fd);
size_t fs_filesz(int fd);

static uintptr_t loader(PCB *pcb, const char *filename) {
  int fd = fs_open(filename, 0, 0);
  fs_read(fd, (void*)DEFAULT_ENTRY, fs_filesz(fd));
  fs_close(fd);
  return DEFAULT_ENTRY;
}

void naive_uload(PCB *pcb, const char *filename) {
  uintptr_t entry = loader(pcb, filename);
  ((void(*)())entry) ();
}

void context_kload(PCB *pcb, void *entry) {
  _Area stack;
  stack.start = pcb->stack;
  stack.end = stack.start + sizeof(pcb->stack);

  pcb->tf = _kcontext(stack, entry, NULL);
}

void context_uload(PCB *pcb, const char *filename) {
  uintptr_t entry = loader(pcb, filename);

  _Area stack;
  stack.start = pcb->stack;
  stack.end = stack.start + sizeof(pcb->stack);

  pcb->tf = _ucontext(&pcb->as, stack, stack, (void *)entry, NULL);
}
