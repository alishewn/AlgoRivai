#include <stdio.h>
#include "log.h"

#define DUMP_SIZE_PER_LINE      (64)

#if (ENABLE_HEX_DUMP)
void dump_hex(uint8_t *addr, uint32_t len, const char *desc)
{
    uint32_t i;

    PRINT_FUNC("\nHex dump[%s]", desc);

    for (i = 0; i < len; i++) {
        if (0 == (i % DUMP_SIZE_PER_LINE))
        {
            PRINT_FUNC("\n");
        }

        PRINT_FUNC("%02x ", addr[i]);
    }

    PRINT_FUNC("\n");
}
#else
void dump_hex(uint8_t *addr, uint32_t len, const char *desc)
{
    (void)(addr);
    (void)(len);
    (void)(desc);
}
#endif

