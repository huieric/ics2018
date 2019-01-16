#include "memory.h"
#include "proc.h"

static void *pf = NULL;

#define MAP_TEST 1
#define MAP_CREATE 2

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
  current->cur_brk = new_brk;
  if (new_brk > current->max_brk) {
    uintptr_t vaddr_start = (current->max_brk / current->as.pgsize) * current->as.pgsize;
    
    if (_map(&current->as, (void *)vaddr_start, NULL, MAP_TEST)) vaddr_start += current->as.pgsize;
    while (vaddr_start < new_brk) {
      void* page_base = new_page(1);
      _map(&current->as, (void *)vaddr_start, page_base, MAP_CREATE);
      vaddr_start += current->as.pgsize;
    }
    current->max_brk = new_brk;
  }
  return 0;
}

void init_mm() {
  pf = (void *)PGROUNDUP((uintptr_t)_heap.start);
  Log("free physical pages starting from %p", pf);

  _vme_init(new_page, free_page);
}
