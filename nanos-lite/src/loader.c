#include "proc.h"

#define DEFAULT_ENTRY 0x8048000

size_t ramdisk_read(void* buf, size_t offset, size_t size);
size_t get_ramdisk_size();
int fs_open(const char* pathname, int flags, int mode);
size_t fs_read(int fd, void* buf, size_t len);
int fs_close(int fd);
size_t fs_filesz(int fd);
void* new_page(size_t nr_page);

static uintptr_t loader(PCB *pcb, const char *filename) {
  int fd = fs_open(filename, 0, 0);
  void* va = (void*)DEFAULT_ENTRY;
  for (int i = 0; i < fs_filesz(fd); i += PGSIZE, va += PGSIZE) {
    void* pg = new_page(1);
    _map(&pcb->as, va, pg, 0x001);
    fs_read(fd, (void*)pg, PGSIZE);
  }  
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
  _protect(&pcb->as);  
  uintptr_t entry = loader(pcb, filename);

  _Area stack;
  stack.start = pcb->stack;
  stack.end = stack.start + sizeof(pcb->stack);  
  pcb->max_brk = pcb->cur_brk = (uintptr_t)stack.start;

  pcb->tf = _ucontext(&pcb->as, stack, stack, (void *)entry, NULL);
}
