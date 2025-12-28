#pragma once

#ifdef __cplusplus
extern "C" {
#endif

extern char ubi_handler;

struct saved_regs {
    unsigned int ssr, spc, r7, r6, r5, r4, r3, r2, r1, r0, pr, macl, mach, r14, r13, r12, r11, r10, r9, r8, r7_bank,
            r6_bank, r5_bank, r4_bank, r3_bank, r2_bank, r1_bank, r0_bank, sgr;
};

// returns true if the bank registers should not be written back
bool ubi_c_handler(struct saved_regs *regs);

extern void *ubi_debug_stack;

#ifdef __cplusplus
}
#endif
