#include <algorithm>
#include <cstdint>
#include <ranges>
//#include <sdk/os/lcd.h>
//#include <sdk/os/debug.h>

#include "ubc.h"
#include "Singleton.hpp"
#include "helper.hpp"

using S = UBI::Singleton;

[[maybe_unused]] static void switch_interrupt_block() {
    unsigned int sr;
    __asm__ ("stc sr, %0" : "=r"(sr));
    sr ^= 1u << 28; // Block interrupt
    __asm__ volatile ("ldc %0, sr" : : "r"(sr));
}

[[maybe_unused]] static void restore_banked_regs(saved_regs *regs) {
    __asm__ volatile ("ldc %0, r7_bank" : : "r"(regs->r7_bank));
    __asm__ volatile ("ldc %0, r6_bank" : : "r"(regs->r6_bank));
    __asm__ volatile ("ldc %0, r5_bank" : : "r"(regs->r5_bank));
    __asm__ volatile ("ldc %0, r4_bank" : : "r"(regs->r4_bank));
    __asm__ volatile ("ldc %0, r3_bank" : : "r"(regs->r3_bank));
    __asm__ volatile ("ldc %0, r2_bank" : : "r"(regs->r2_bank));
    __asm__ volatile ("ldc %0, r1_bank" : : "r"(regs->r1_bank));
    __asm__ volatile ("ldc %0, r0_bank" : : "r"(regs->r0_bank));
}

bool ubi_c_handler(saved_regs *regs [[maybe_unused]]) {
    AS_STRUCT_TYPE(UBC_CCMFR) mfr;
    AS_STRUCT_GET(UBC_CCMFR, mfr);

    /*static unsigned int caught = 0;
    caught++;
    Debug_Printf(0, 20, false, 0, "Status Registers:");
    Debug_Printf(0, 21, false, 0, "  spc: 0x%08x  ssr: 0x%08x   pr: 0x%08x  ", regs->spc, regs->ssr, regs->pr);
    Debug_Printf(0, 22, false, 0, " macl: 0x%08x mach: 0x%08x                   ", regs->macl, regs->mach);
    Debug_Printf(0, 24, false, 0, "Bank 0:");
    Debug_Printf(0, 25, false, 0, "   r0: 0x%08x   r1: 0x%08x   r2: 0x%08x  ", regs->r0, regs->r1, regs->r2);
    Debug_Printf(0, 26, false, 0, "   r3: 0x%08x   r4: 0x%08x   r5: 0x%08x  ", regs->r3, regs->r4, regs->r5);
    Debug_Printf(0, 27, false, 0, "   r6: 0x%08x   r7: 0x%08x                   ", regs->r6, regs->r7);
    Debug_Printf(0, 29, false, 0, "Bank 1:");
    Debug_Printf(0, 30, false, 0, "   r0: 0x%08x   r1: 0x%08x   r2: 0x%08x  ", regs->r0_bank, regs->r1_bank, regs->r2_bank);
    Debug_Printf(0, 31, false, 0, "   r3: 0x%08x   r4: 0x%08x   r5: 0x%08x  ", regs->r3_bank, regs->r4_bank, regs->r5_bank);
    Debug_Printf(0, 32, false, 0, "   r6: 0x%08x   r7: 0x%08x                   ", regs->r6_bank, regs->r7_bank);
    Debug_Printf(0, 34, false, 0, "Non-Banked:");
    Debug_Printf(0, 35, false, 0, "   r8: 0x%08x   r9: 0x%08x  r10: 0x%08x  ", regs->r8, regs->r9, regs->r10);
    Debug_Printf(0, 36, false, 0, "  r11: 0x%08x  r12: 0x%08x  r13: 0x%08x  ", regs->r11, regs->r12, regs->r13);
    Debug_Printf(0, 37, false, 0, "  r14: 0x%08x  r15: 0x%08x                   ", regs->r14, reinterpret_cast<std::uintptr_t>(regs) - sizeof(*regs));
    Debug_Printf(4, 39, false, 0, "MF0: %u   MF1: %u   C: %5u", mfr.mf0, mfr.mf1, caught);
    LCD_Refresh();*/

    if (!(mfr.mf0 || mfr.mf1)) {
        AS_STRUCT_SET(UBC_CCMFR, {false, false});
        dummy_icbi();
        return false;
    }

    const auto &handler_array = mfr.mf0 ? S::instance.computed_channel0_array : S::instance.computed_channel1_array;

    auto [first, last] = std::equal_range(
        handler_array.begin(),
        handler_array.end(),
        std::make_pair(reinterpret_cast<volatile void *>(regs->spc), nullptr), // handler value unused
        [](const auto &a, const auto &b) { return a.first < b.first; }
    );
    auto handlers_view = std::ranges::subrange(first, last) | std::views::transform([](const auto &entry) {
        return entry.second;
    });

    if (mfr.mf0) // written like bitwise and
        AS_STRUCT_SET(UBC_CCMFR, {true, false});
    else
        AS_STRUCT_SET(UBC_CCMFR, {false, true});
    dummy_icbi();

    if (handlers_view.empty())
        return false;

    restore_banked_regs(regs);
    switch_interrupt_block();

    UBI::HandlerContext ctx;
    for (const auto handler: handlers_view) {
        handler(ctx);
    }

    switch_interrupt_block();
    return true;
}
