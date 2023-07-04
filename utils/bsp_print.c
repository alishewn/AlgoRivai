// See LICENSE for license details.
#include <stdarg.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include "atomic.h"
#include "bsp_print.h"
#include "common.h"

#define SPINLOCK_INIT       {0}
#define SYS_write           (64)

struct putchar_data {
    char *buf;
    int bufsize;
    int buflen;
    int end;
};


static spinlock_t syscalls_lock __attribute__((section(".spinlock"))) = SPINLOCK_INIT;

static uint64_t zeroExtend(long val);
static int putchar_c(int ch, unsigned long putdat);
static inline void printnum(void (*putch)(int, unsigned long), unsigned long putdat, unsigned long long num, unsigned base, int width, int padc);
static unsigned long long getuint(va_list *ap, int lflag);
static long long getint(va_list *ap, int lflag);
static void vFormatPrintString(void (*putch)(int, unsigned long), unsigned long putdat, const char *fmt, va_list ap);

static uint64_t zeroExtend(long val)
{
    uint64_t ret = val;

#if __riscv_xlen == 32
    ret = (0x00000000ffffffff & val);
#endif

    return ret;
}

static int putchar_c(int ch, unsigned long putdat)
{
    struct putchar_data *pdata = (struct putchar_data *)putdat;
    char *buf = pdata->buf;

    if (ch != '\0') {
        buf[pdata->buflen++] = ch;
    }

    if ((pdata->buflen == pdata->bufsize - 1) || ((0 != pdata->end) && (ch == '\0'))) {
        vSyscallToHost(SYS_write, 1, (long) buf, pdata->buflen);
        pdata->buflen = 0;
    }

    return 0;
}

/* Writes number to putchar. */
static inline void printnum(void (*putch)(int, unsigned long), unsigned long putdat, unsigned long long num, unsigned base, int width, int padc)
{
    unsigned digs[sizeof(num) * CHAR_BIT];
    int pos = 0;

    for(;;) {
        digs[pos++] = num % base;
        if (num < base) {
            break;
        }
        num /= base;
    }

    while (width-- > pos){
        putch(padc, putdat);
    }

    while (pos-- > 0){
        putch(digs[pos] + (digs[pos] >= 10 ? 'a' - 10 : '0'), putdat);
    }
}

/* Returns unsigned integer from argument list. */
static unsigned long long getuint(va_list *ap, int lflag)
{
    if (lflag >= 2) {
        return va_arg(*ap, unsigned long long);
    } else if (lflag) {
        return va_arg(*ap, unsigned long);
    } else {
        return va_arg(*ap, unsigned int);
    }
}

/* Returns signed integer from argument list. */
static long long getint(va_list *ap, int lflag)
{
    if (lflag >= 2) {
        return va_arg(*ap, long long);
    } else if (lflag) {
        return va_arg(*ap, long);
    } else {
        return va_arg(*ap, int);
    }
}

/* Format and print a string. */
static void vFormatPrintString(void (*putch)(int, unsigned long), unsigned long putdat, const char *fmt, va_list ap)
{
    register const char* p;
    const char* last_fmt;
    register int ch;
    unsigned long long num;
    int base, lflag, width, precision;
    char padc;

    while (1) {
        while ((ch = *(unsigned char *) fmt) != '%') {
            putch(ch, putdat);
            if (ch == '\0') {
                return;
            }
            fmt++;
        }
        fmt++;

        // Process a %-escape sequence
        last_fmt = fmt;
        padc = ' ';
        width = -1;
        precision = -1;
        lflag = 0;
        reswitch: switch (ch = *(unsigned char *) fmt++) {
            /* flag to pad on the right */
            case '-':
                padc = '-';
                goto reswitch;

                /* flag to pad with 0's instead of spaces */
            case '0':
                padc = '0';
                goto reswitch;

                /* width field */
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
                for (precision = 0;; ++fmt) {
                    precision = precision * 10 + ch - '0';
                    ch = *fmt;
                    if (ch < '0' || ch > '9')
                        break;
                }
                goto process_precision;

            case '*':
                precision = va_arg(ap, int);
                goto process_precision;

            case '.':
                if (width < 0)
                    width = 0;
                goto reswitch;

            case '#':
                goto reswitch;

                process_precision: if (width < 0)
                    width = precision, precision = -1;
                goto reswitch;

                /* long flag (doubled for long long) */
            case 'l':
                lflag++;
                goto reswitch;

                /* character */
            case 'c':
                putch(va_arg(ap, int), putdat);
                break;

            /* string */
            case 's':
                if ((p = va_arg(ap, char *)) == NULL)
                    p = "(null)";
                if (width > 0 && padc != '-')
                    for (width -= strnlen(p, precision); width > 0; width--)
                        putch(padc, putdat);
                for (; (ch = *p) != '\0' && (precision < 0 || --precision >= 0); width--) {
                    putch(ch, putdat);
                    p++;
                }
                for (; width > 0; width--)
                    putch(' ', putdat);
                break;

            /* (signed) decimal */
            case 'd':
                num = getint(&ap, lflag);
                if ((long long) num < 0) {
                    putch('-', putdat);
                    num = -(long long) num;
                }
                base = 10;
                goto signed_number;

                /* unsigned decimal */
            case 'u':
                base = 10;
                goto unsigned_number;

                /* (unsigned) octal */
            case 'o':
                base = 8;
                goto unsigned_number;

#if 0
            /* pointer */
            case 'p':
                static_assert(sizeof(long) == sizeof(void*))
                ;
                lflag = 1;
                putch('0', putdat);
                putch('x', putdat);
#endif
            /* no break, fall through to hexidecimal */
            /* (unsigned) hexadecimal */
            case 'x':
            case 'X': // TODO:
                base = 16;
                unsigned_number: num = getuint(&ap, lflag);
                signed_number: printnum(putch, putdat, num, base, width, padc);
                break;

                /* escaped '%' character */
            case '%':
                putch(ch, putdat);
                break;

                /* unrecognized escape sequence */
            default:
                putch('%', putdat);
                fmt = last_fmt;
                break;
        }
    }
}

uint64_t vSyscallToHost(long which, long arg0, long arg1, long arg2)
{
    spinlock_lock(&syscalls_lock); //aquire spinlock

    volatile uint64_t magic_mem[8] __attribute__((aligned(64)));

    magic_mem[0] = zeroExtend(which);
    magic_mem[1] = zeroExtend(arg0);
    magic_mem[2] = zeroExtend(arg1);
    magic_mem[3] = zeroExtend(arg2);

    __sync_synchronize();
    tohost = zeroExtend((long)magic_mem);
    __sync_synchronize();
    waiting_fromhost();
    __sync_synchronize();

    fromhost = 0;
    __sync_synchronize();
    spinlock_unlock(&syscalls_lock); //release spinlock

    return magic_mem[0];
}

int printj(const char* fmt, ...)
{
    char buf[128];
    struct putchar_data pdata;

    pdata.buf = buf;
    pdata.bufsize = sizeof(buf);
    pdata.buflen = 0;
    pdata.end = 1;

    va_list ap;
    va_start(ap, fmt);

    vFormatPrintString((void*) putchar_c, (unsigned long)&pdata, fmt, ap);

    va_end(ap);
    return 0; // incorrect return value, but who cares, anyway?
}

