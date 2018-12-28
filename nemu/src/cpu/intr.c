#include "cpu/exec.h"
#include "memory/mmu.h"

void raise_intr(uint8_t NO, vaddr_t ret_addr) {
  /* TODO: Trigger an interrupt/exception with ``NO''.
   * That is, use ``NO'' to index the IDT.
   */
  rtl_push(&cpu.eflags);
  t0 = cpu.cs;
  rtl_push(&t0);
  rtl_push(&ret_addr);

  GateDesc gatedesc;
  vaddr_t addr = cpu.idtr.base + NO * sizeof(GateDesc);
  gatedesc.valL = vaddr_read(addr, 4);
  gatedesc.valH = vaddr_read(addr + 4, 4);
  Log("NO=%x", NO);
  Assert(gatedesc.present, "invalid gate descriptor!");

  decoding.jmp_eip = (gatedesc.offset_31_16 << 16) | gatedesc.offset_15_0;
  rtl_j(decoding.jmp_eip);
}

void dev_raise_intr() {
}
