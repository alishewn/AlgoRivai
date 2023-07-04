#ifndef __COMMON_H__
#define __COMMON_H__

#include <stdint.h>

#define FINISH(a) \
    asm volatile("fence \n\t"); \
    tohost = 1 | ((int32_t)0x##a << 1); \
    asm volatile("fence \n\t"); \
    asm volatile("csrc mstatus, 8 \n\t"); \
    asm volatile("jal waiting_fromhost"); \
    asm volatile("fence \n\t"); \
    fromhost = 0; \
    asm volatile("fence \n\t"); \
    asm volatile("wfi \n\t"); \

#endif

extern volatile uint64_t tohost;
extern volatile uint64_t fromhost;

void waiting_fromhost(void);
unsigned int read_cycle(void);