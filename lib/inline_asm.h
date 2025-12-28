#pragma once

inline __attribute__((always_inline)) void switch_interrupt_block() {
    unsigned int sr;
    __asm__ ("stc sr, %0" : "=r"(sr));
    sr ^= 1u << 28; // Block interrupt
    __asm__ volatile ("ldc %0, sr" : : "r"(sr));
}
