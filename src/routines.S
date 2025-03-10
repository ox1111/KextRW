.align 4
.global _arm_kvtophys, _arbitrary_call

_arm_kvtophys:
    bti c
    mrs x2, DAIF
    msr DAIFSet, #0xF

    at s1e1r, x0
    isb sy
    mrs x1, PAR_EL1
    msr DAIF, x2

    tbnz x1, #0, L_kvtop_invalid
    bfm x1, x0, #0, #11
    and x0, x1, #0x0000ffffffffffff
    ret

L_kvtop_invalid:
    mov x0, #0
    ret

_arbitrary_call:
    pacibsp
    ldp     x8, x9, [sp]
    sub     sp, sp, #0x40
    stp     fp, lr, [sp, #0x10]
    stp     x19, x20, [sp, #0x30]

    xpacd   x9
    xpaci   x8
    stur    x9, [sp, #0x20]
    add     fp, sp, #0x20

    // No point signing, compiler will optimise it out anyway
    blr     x8

    ldp     x19, x20, [sp, #0x30]
    ldp     fp, lr, [sp, #0x10]
    ldur    x8, [sp, #0x20]
    add     sp, sp, #0x40

    cbz     x8, 0f
    stur    x0, [x8]
0:
    retab