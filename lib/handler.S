#define _CONCAT(x, y) x##y
#define CONCAT(x, y) _CONCAT(x, y)
#define CSYM(name) CONCAT(__USER_LABEL_PREFIX__, name)

.section .text.ubi_handler, "ax"
.global CSYM(ubi_handler)
.type CSYM(ubi_handler), @function
.align 2
CSYM(ubi_handler):
    mov.l .L2, r15
    tst r15, r15
    bt .L3
    stc sgr, r15
.L3:
    stc.l sgr, @-r15
    mov.l r0, @-r15
    mov.l r1, @-r15
    mov.l r2, @-r15
    mov.l r3, @-r15
    mov.l r4, @-r15
    mov.l r5, @-r15
    mov.l r6, @-r15
    mov.l r7, @-r15
    mov.l r8, @-r15
    mov.l r9, @-r15
    mov.l r10, @-r15
    mov.l r11, @-r15
    mov.l r12, @-r15
    mov.l r13, @-r15
    mov.l r14, @-r15
    sts.l mach, @-r15
    sts.l macl, @-r15
    sts.l pr, @-r15
    stc.l r0_bank, @-r15
    stc.l r1_bank, @-r15
    stc.l r2_bank, @-r15
    stc.l r3_bank, @-r15
    stc.l r4_bank, @-r15
    stc.l r5_bank, @-r15
    stc.l r6_bank, @-r15
    stc.l r7_bank, @-r15
    stc.l spc, @-r15
    stc.l ssr, @-r15

    mov #29, r12
    mov #1, r10
    shld r12, r10
    stc sr, r11
    xor r10, r11
    ldc r11, sr

    mov.l .L1, r1
    jsr @r1
    mov r15, r4
    tst r0, r0

    ldc.l @r15+, ssr
    ldc.l @r15+, spc
    mov.l @r15+, r7
    mov.l @r15+, r6
    mov.l @r15+, r5
    mov.l @r15+, r4
    mov.l @r15+, r3
    mov.l @r15+, r2
    mov.l @r15+, r1
    mov.l @r15+, r0
    lds.l @r15+, pr
    lds.l @r15+, macl
    lds.l @r15+, mach
    mov.l @r15+, r14
    mov.l @r15+, r13
    mov.l @r15+, r12
    mov.l @r15+, r11
    mov.l @r15+, r10
    mov.l @r15+, r9
    mov.l @r15+, r8
    bf .L4
    add #32, r15
    rte
    mov.l @r15, r15
.L4:
    ldc.l @r15+, r7_bank
    ldc.l @r15+, r6_bank
    ldc.l @r15+, r5_bank
    ldc.l @r15+, r4_bank
    ldc.l @r15+, r3_bank
    ldc.l @r15+, r2_bank
    ldc.l @r15+, r1_bank
    ldc.l @r15+, r0_bank
    rte
    mov.l @r15, r15
.align 4
.L1:
    .long CSYM(ubi_c_handler)
.L2:
    .long CSYM(ubi_debug_stack)
.size CSYM(ubi_handler), .-CSYM(ubi_handler)