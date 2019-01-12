#include "memory.h"
#include "proc.h"

static void *pf = NULL;

void* new_page(size_t nr_page) {
  void *p = pf;
  pf += PGSIZE * nr_page;
  assert(pf < (void *)_heap.end);
  return p;
}

void free_page(void *p) {
  panic("not implement yet");
}

/* The brk() system call handler. */
int mm_brk(uintptr_t new_brk) {
  if (current->max_brk == 0) {
    current->max_brk = current->cur_brk = new_brk; //init in first call
  }
  uintptr_t brk = current->max_brk;
  for (; brk < new_brk; brk += PGSIZE) {
    void* pa = new_page(1);
    _map(&current->as, (void*)brk, pa, 0x001);
  }
  current->max_brk = brk;
  current->cur_brk = new_brk;
  Log("cur_brk=%p", current->cur_brk);
  return 0;
}

void init_mm() {
  pf = (void *)PGROUNDUP((uintptr_t)_heap.start);
  Log("free physical pages starting from %p", pf);

  _vme_init(new_page, free_page);
}
