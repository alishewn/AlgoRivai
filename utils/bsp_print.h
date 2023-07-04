#ifndef __BSP_PRINT_H__
#define __BSP_PRINT_H__

#include <stdint.h>

uint64_t vSyscallToHost(long which, long arg0, long arg1, long arg2);
int printj(const char* fmt, ...);

#endif

