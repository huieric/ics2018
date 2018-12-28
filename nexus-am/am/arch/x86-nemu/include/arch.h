#ifndef __ARCH_H__
#define __ARCH_H__

#include <am.h>

#define PMEM_SIZE (128 * 1024 * 1024)
#define PGSIZE    4096    // Bytes mapped by a page

typedef uint32_t size_t;

struct _Context {
  uintptr_t eflags, cs, eip;
  uintptr_t err;
  int       irq;
  uintptr_t eax, ecx, edx, ebx, esp, ebp, esi, edi;
  struct _Protect *prot;
  };

#define GPR1 eax
#define GPR2 eip
#define GPR3 eip
#define GPR4 eip
#define GPRx eip

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif
#endif
