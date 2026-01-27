#include "HandlerContext.hpp"
#include "inline_asm.h"

using namespace UBI;

HandlerContext::HandlerContext(const saved_regs &saved_regs) noexcept : registers{
    .unbanked = {
        Register(saved_regs.r8),
        Register(saved_regs.r9),
        Register(saved_regs.r10),
        Register(saved_regs.r11),
        Register(saved_regs.r12),
        Register(saved_regs.r13),
        Register(saved_regs.r14),
        Register(saved_regs.sgr)
    },
    .banked = {
        {
            Register(saved_regs.r0),
            Register(saved_regs.r1),
            Register(saved_regs.r2),
            Register(saved_regs.r3),
            Register(saved_regs.r4),
            Register(saved_regs.r5),
            Register(saved_regs.r6),
            Register(saved_regs.r7)
        },
        {
            Register(saved_regs.r0_bank),
            Register(saved_regs.r1_bank),
            Register(saved_regs.r2_bank),
            Register(saved_regs.r3_bank),
            Register(saved_regs.r4_bank),
            Register(saved_regs.r5_bank),
            Register(saved_regs.r6_bank),
            Register(saved_regs.r7_bank)
        }
    },
    .SR = Register(saved_regs.ssr),
    .PC = Register(saved_regs.spc),
    .PR = Register(saved_regs.pr),
    .MACL = Register(saved_regs.macl),
    .MACH = Register(saved_regs.mach)
} {
}

void HandlerContext::writeBack(saved_regs &saved_regs) const noexcept {
    saved_regs = {
        .ssr = registers.SR,
        .spc = registers.PC,
        .r7 = registers.banked[0][7],
        .r6 = registers.banked[0][6],
        .r5 = registers.banked[0][5],
        .r4 = registers.banked[0][4],
        .r3 = registers.banked[0][3],
        .r2 = registers.banked[0][2],
        .r1 = registers.banked[0][1],
        .r0 = registers.banked[0][0],
        .pr = registers.PR,
        .macl = registers.MACL,
        .mach = registers.MACH,
        .r14 = registers.unbanked[6],
        .r13 = registers.unbanked[5],
        .r12 = registers.unbanked[4],
        .r11 = registers.unbanked[3],
        .r10 = registers.unbanked[2],
        .r9 = registers.unbanked[1],
        .r8 = registers.unbanked[0],
        .r7_bank = registers.banked[1][7],
        .r6_bank = registers.banked[1][6],
        .r5_bank = registers.banked[1][5],
        .r4_bank = registers.banked[1][4],
        .r3_bank = registers.banked[1][3],
        .r2_bank = registers.banked[1][2],
        .r1_bank = registers.banked[1][1],
        .r0_bank = registers.banked[1][0],
        .sgr = registers.unbanked[7],
    };
}

void HandlerContext::writeBankedRegisters() const noexcept {
    __asm__ volatile ("ldc %0, r7_bank" : : "r"(registers.banked[1][7].asCopy<unsigned int>()));
    __asm__ volatile ("ldc %0, r6_bank" : : "r"(registers.banked[1][6].asCopy<unsigned int>()));
    __asm__ volatile ("ldc %0, r5_bank" : : "r"(registers.banked[1][5].asCopy<unsigned int>()));
    __asm__ volatile ("ldc %0, r4_bank" : : "r"(registers.banked[1][4].asCopy<unsigned int>()));
    __asm__ volatile ("ldc %0, r3_bank" : : "r"(registers.banked[1][3].asCopy<unsigned int>()));
    __asm__ volatile ("ldc %0, r2_bank" : : "r"(registers.banked[1][2].asCopy<unsigned int>()));
    __asm__ volatile ("ldc %0, r1_bank" : : "r"(registers.banked[1][1].asCopy<unsigned int>()));
    __asm__ volatile ("ldc %0, r0_bank" : : "r"(registers.banked[1][0].asCopy<unsigned int>()));
}

void HandlerContext::readBankedRegisters() noexcept {
    unsigned int r0, r1, r2, r3, r4, r5, r6, r7;

    __asm__ ("ldc %0, r7_bank" : "=r"(r7));
    __asm__ ("ldc %0, r6_bank" : "=r"(r6));
    __asm__ ("ldc %0, r5_bank" : "=r"(r5));
    __asm__ ("ldc %0, r4_bank" : "=r"(r4));
    __asm__ ("ldc %0, r3_bank" : "=r"(r3));
    __asm__ ("ldc %0, r2_bank" : "=r"(r2));
    __asm__ ("ldc %0, r1_bank" : "=r"(r1));
    __asm__ ("ldc %0, r0_bank" : "=r"(r0));

    registers.banked[1][7] = r7;
    registers.banked[1][6] = r6;
    registers.banked[1][5] = r5;
    registers.banked[1][4] = r4;
    registers.banked[1][3] = r3;
    registers.banked[1][2] = r2;
    registers.banked[1][1] = r1;
    registers.banked[1][0] = r0;
}

void HandlerContext::allowNestedInterrupts() noexcept {
    if (nesting_active) return;
    nesting_active = true;

    writeBankedRegisters();

    switch_interrupt_block();
}

void HandlerContext::disallowNestedInterrupts() noexcept {
    if (!nesting_active) return;
    nesting_active = false;

    switch_interrupt_block();

    readBankedRegisters();
}
