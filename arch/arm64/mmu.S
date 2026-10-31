    .section ".text","ax"
    .global arch_mmu_enable
arch_mmu_enable:
    mov x0, #0xFF
    msr mair_el1, x0
    isb
    ldr x1, =__l0_tt
    ldr x2, =__l1_tt
    orr x2, x2, #0x3
    str x2, [x1]
    ldr x1, =__l1_tt
    mov x3, #0
    mov x4, #16
    ldr x2, =0x0000000000000705
1:  lsl x5, x3, #30
    orr x5, x5, x2
    str x5, [x1, x3, lsl #3]
    add x3, x3, #1
    cmp x3, x4
    b.lt 1b
    ldr x0, =0x0000000000083519
    msr tcr_el1, x0
    isb
    ldr x0, =__l0_tt
    msr ttbr0_el1, x0
    msr ttbr1_el1, x0
    isb
    tlbi vmalle1
    dsb sy
    isb
    mrs x0, sctlr_el1
    orr x0, x0, #(1<<0)
    orr x0, x0, #(1<<2)
    orr x0, x0, #(1<<12)
    msr sctlr_el1, x0
    isb
    ret
    .section ".bss","aw",%nobits
    .align 12
__l0_tt: .space 4096
__l1_tt: .space 4096
