#include <stdint.h>
#include <stdio.h>

#define read_seg(_reg_)                                 \
  ({                                                    \
    uint16_t seg;                                       \
    asm volatile ("movw %%"#_reg_", %%ax":"=a"(seg));   \
    seg;                                                \
  })

#define read_cs() read_seg(cs)
#define read_ds() read_seg(ds)
#define read_es() read_seg(es)
#define read_fs() read_seg(fs)
#define read_gs() read_seg(gs)
#define read_ss() read_seg(ss)

void SelectorPrint(char* regName, unsigned short reg){
 printf("U_SegRegs: %s Register = 0x%02x\n", regName, reg);
 printf("U_SegRegs: %s Register RL = %x, Table Indicator = %s, Index = 0x%04x\n", regName, (reg & 3), ((reg >> 2) & 1) ? "1 (LDT)" : "0 (GDT)", ((reg >> 3) & 0x1FFF)); 
}

int main(){
  uint16_t cs = 0, ds = 0, ss = 0, es = 0, fs = 0, gs = 0;

  cs = read_cs();
  ss = read_ss();
  ds = read_ds();
  es = read_es();
  fs = read_fs();
  gs = read_gs();

  SelectorPrint("CS", cs);
  SelectorPrint("DS", ds);
  SelectorPrint("ES", es);
  SelectorPrint("FS", fs);
  SelectorPrint("GS", gs);
  SelectorPrint("SS", ss);

  return 0x0ddba11;
}
