#include "nemu.h"

const char *regsl[] = {
  "zero", "ra", "sp", "gp", "tp", "t0", "t1", "t2",
  "s0", "s1", "a0", "a1", "a2", "a3", "a4", "a5",
  "a6", "a7", "s2", "s3", "s4", "s5", "s6", "s7",
  "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6"
};

void isa_reg_display() {
  for (int i = 0; i < 32; i++) {
    printf("%s=0x%x(%d)\t", regsl[i], cpu.gpr[i]._32, cpu.gpr[i]._32);
    if (i % 4 == 3) printf("\n");
  }
  printf("pc=0x%x(%u)\n", cpu.pc, cpu.pc);
}

uint32_t isa_reg_str2val(const char *s, bool *success) {
  for (int i = 0; i < 32; i++) {
    if (!strcmp(regsl[i], s)) {
      *success = true;
      return cpu.gpr[i]._32;
    }
  }
  if (!strcmp("pc", s)) {
    *success = true;
    return cpu.pc;
  }
  *success = false;
  return 0;
}
