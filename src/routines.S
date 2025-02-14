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
    ldur x8, [sp]
    xpaci x8
    blr x8