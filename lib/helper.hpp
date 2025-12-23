#pragma once

[[gnu::always_inline]] inline void dummy_icbi() {
    __asm__ volatile ("icbi %0" : : "m"(*reinterpret_cast<char *>(0x8000000)));
}
