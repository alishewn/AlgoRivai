#include "common.h"

volatile uint64_t tohost __attribute__((section(".htif")));
volatile uint64_t fromhost __attribute__((section(".htif")));

void __attribute__ ((optimize ("O0"))) waiting_fromhost( void )
{
    while (fromhost == 0);
}

#pragma GCC push_options
#pragma GCC optimize("O3")
unsigned int read_cycle(void)
{
    unsigned int cycles=0;

    asm volatile ("rdcycle %0" : "=r" (cycles) :);

    return cycles;
}
#pragma GCC pop_options