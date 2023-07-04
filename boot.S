    .section ".text.init"
    .globl _start
_start:
    j    do_reset

    .align 2
mtrap_vec_tab:
    .align 2
    j trap_entry
    .align 2
    j reserved_cause
    .align 2
    j reserved_cause
    .align 2
    j m_sorft_int_handler
    .fill 8, 4, 0xdeadbeef

trap_init:
    // MPIE = 1, MPP = 00
    // li t0, 0x80

    // MPIE = 1, MPP = 11
    // li t0, 0x1880

    // MPIE = 1, MPP = 11, FS = 01, VS = 01
    li t0, 0x3A80
    csrrw t0, mstatus, t0

    // Set trap entry and set vector model
    la t0, mtrap_vec_tab
    addi t0, t0, 1
    csrrw t0, mtvec, t0

    // Set mepc
    la t0, main
    csrrw t0, mepc, t0

    // Set MSIE
    // csrrsi x0, mie, 0x08

    // Return to user model
    mret

trap_entry:
    nop
    csrr t0, mcause

    // Adjust mepc to execute next instruction
    csrr t0, mepc
    addi t0, t0, 4
    csrrw t0, mepc, t0

    mret

reserved_cause:
1:  nop
    j 1b

m_sorft_int_handler:
    nop
    csrr t0, mcause
    csrr t0, mepc

    // Clean MIP
    csrwi mip, 0
    mret

do_reset:
    li   x1, 0
    li   x2, 0
    li   x3, 0
    li   x3, 0
    li   x4, 0
    li   x5, 0
    li   x6, 0
    li   x7, 0
    li   x8, 0
    li   x9, 0
    li   x10, 0
    li   x11, 0
    li   x12, 0
    li   x13, 0
    li   x14, 0
    li   x15, 0
    li   x16, 0
    li   x17, 0
    li   x18, 0
    li   x19, 0
    li   x20, 0
    li   x21, 0
    li   x22, 0
    li   x23, 0
    li   x24, 0
    li   x25, 0
    li   x26, 0
    li   x27, 0
    li   x28, 0
    li   x29, 0
    li   x30, 0
    li   x31, 0

    la   a0, _edata
    la   a2, _end
    sub  a2, a2, a0
    li   a1, 0
    //call memset

    la   sp, init_sp
    j    trap_init
