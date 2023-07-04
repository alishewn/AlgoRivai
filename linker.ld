OUTPUT_ARCH( "riscv" )
ENTRY(_start)

MEMORY
{
    RAM : ORIGIN = 0x80000000, LENGTH = 0x8000000
}


SECTIONS
{
    .text.init :
    {
        *(.text.init)
    } > RAM

    .htif ALIGN(0x10) :
    {
        *(.htif)
    } > RAM

    .text ALIGN(0x10) :
    {
        *(.text)
    } > RAM

    .inst_code ALIGN(0x10) :
    {
        *(.inst_code)
    } > RAM

    .data :
    {
        *(.sdata)
        *(.data)
        fill_start = .;
        . = . + 0x10;
        fill_end = .;
        LONG(0xdeaddead)
    } > RAM =0x12345678

    _edata = .;
    .bss ALIGN(0x10) :
    {
        *(.bss)
    } > RAM
    _end = .;

    . = ALIGN(0x10);
    init_sp = . + 0x200000;
}

