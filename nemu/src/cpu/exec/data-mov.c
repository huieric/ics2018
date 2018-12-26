#include "cpu/exec.h"

make_EHelper(mov) {
  operand_write(id_dest, &id_src->val);
  print_asm_template2(mov);
}

make_EHelper(push) {
  rtl_push(&id_dest->val);
  print_asm_template1(push);
}

make_EHelper(pop) {
  rtl_pop(&t0);
  operand_write(id_dest, &t0);

  print_asm_template1(pop);
}

make_EHelper(pusha) {
  TODO();

  print_asm("pusha");
}

make_EHelper(popa) {
  TODO();

  print_asm("popa");
}

make_EHelper(leave) {
  int width = decoding.is_operand_size_16 ? 2 : 4;
  rtl_lr(&t0, R_EBP, width);
  rtl_sr(R_ESP, &t0, width);
  rtl_pop(&t0);
  rtl_sr(R_EBP, &t0, width);

  print_asm("leave");
}

make_EHelper(cltd) {
  if (decoding.is_operand_size_16) {
    rtl_lr(&t0, R_AL, 1);
    rtl_msb(&t1, &t0, 1);
    t3 = t1 == 0 ? 0 : ~0;
    rtl_mv(&t2, &t3);
    rtl_sr(R_AH, &t2, 1);
  }
  else {
    rtl_lr(&t0, R_AX, 2);
    rtl_msb(&t1, &t0, 2);
    t3 = t1 ? (t0 | 0xffff0000) : t0;
    rtl_mv(&t2, &t3);
    rtl_sr(R_EAX, &t2, 4);
  }

  print_asm(decoding.is_operand_size_16 ? "cwtl" : "cltd");
}

make_EHelper(cwtl) {
  if (decoding.is_operand_size_16) {
    rtl_lr(&t0, R_AX, 2);
    rtl_msb(&t1, &t0, 2);
    t3 = t1 == 0 ? 0 : ~0;
    rtl_mv(&t2, &t3);
    rtl_sr(R_DX, &t2, 2);
  }
  else {
    rtl_lr(&t0, R_EAX, 4);
    rtl_msb(&t1, &t0, 4);
    t3 = t1 == 0 ? 0 : ~0;
    rtl_mv(&t2, &t3);
    rtl_sr(R_EDX, &t2, 4); 
  }
  
  print_asm(decoding.is_operand_size_16 ? "cbtw" : "cwtl");
}

make_EHelper(movsx) {
  id_dest->width = decoding.is_operand_size_16 ? 2 : 4;
  rtl_sext(&t0, &id_src->val, id_src->width);
  operand_write(id_dest, &t0);
  print_asm_template2(movsx);
}

make_EHelper(movzx) {
  id_dest->width = decoding.is_operand_size_16 ? 2 : 4;
  operand_write(id_dest, &id_src->val);
  print_asm_template2(movzx);
}

make_EHelper(lea) {
  operand_write(id_dest, &id_src->addr);
  print_asm_template2(lea);
}
