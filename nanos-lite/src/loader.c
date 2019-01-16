#include "proc.h"

#define DEFAULT_ENTRY 0x8048000
#define MAP_TEST 1
#define MAP_CREATE 2

size_t ramdisk_read(void* buf, size_t offset, size_t size);
size_t get_ramdisk_size();
int fs_open(const char* pathname, int flags, int mode);
size_t fs_read(int fd, void* buf, size_t len);
int fs_close(int fd);
size_t fs_filesz(int fd);
void* new_page(size_t nr_page);

static uintptr_t loader(PCB *pcb, const char *filename) {
  int fd = fs_open(filename, 0, 0);
  int len = fs_filesz(fd);
  int blen = pcb->as.pgsize;
  
  uintptr_t s = DEFAULT_ENTRY;
  Log("len=0x%x\n", len);
  char buf[blen];
  while (len > 0) {
    void* page_base = new_page(1);
    Log("page_base=%p s=%p\n", page_base, s);
    _map(&pcb->as, (void *)s, page_base, MAP_CREATE);
    fs_read(fd, buf, blen);
    memcpy(page_base, buf , blen);
    s += blen;
    len -= blen;
  }
  pcb->cur_brk = pcb->max_brk = s;
  Log("cur_brk=%p s=%p\n", pcb->cur_brk, s);
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

  pcb->tf = _ucontext(&pcb->as, stack, stack, (void *)entry, NULL);
}
