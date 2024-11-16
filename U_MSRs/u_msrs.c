#include <stdio.h>
#include <stdint.h>

void cpuid(int CPUINFO[4], int functionId, int subFunctionId){
  __asm__ __volatile__(
    "cpuid"
    : "=a"(CPUINFO[0]), "=b"(CPUINFO[1]), "=c"(CPUINFO[2]), "=d"(CPUINFO[3])
    : "a"(functionId), "c"(subFunctionId)
  );
}

uint64_t readmsr(uint32_t msr){
  uint32_t low, high;

  __asm__ __volatile__(
    "rdmsr"
    : "=a"(low), "=d"(high)
    : "c"(msr)
  );

  return ((uint64_t)high << 32) | low;
}

int main(int argc, char* argv[]){

  int cpuInfo[4] = { 0 };
  cpuid(cpuInfo, 7, 0);

  if(((cpuInfo[3] >> 20) & 1) || ((cpuInfo[3] >> 29) & 1)) {
    uint64_t out = readmsr(0xC0000080);
    printf("IA32_EFER = 0x%016lx\n", out);
    if ((out >> 8) & 1) printf("IA32_EFER.LME = 1\n");
    if ((out >> 10) & 1) printf("IA32_EFER.LNA = 1\n");
  } else {
    printf("IA32_EFER not supported on this platform\n");
  }


  return 0;
}
