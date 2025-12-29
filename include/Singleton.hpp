#pragma once
#include <cstdint>
#include <vector>

#include "HandlerContext.hpp"
#include "c_handler.h"

namespace UBI {
    enum class PCBreakpoint : bool {
        BeforeExecution = false,
        AfterExecution = true
    };

    class Breakpoint {
    public:
        volatile void *address;
        PCBreakpoint pc_breakpoint = PCBreakpoint::AfterExecution;
        volatile void *spc = pc_breakpoint == PCBreakpoint::AfterExecution
                                 ? static_cast<volatile std::uint16_t *>(address) + 1
                                 : address;
        handler_function handler;
        //negative to positive; equal unspecified
        int priority = 0;

        Breakpoint(volatile void *address, handler_function handler, int priority) noexcept;

        Breakpoint(volatile void *address, volatile void *spc, handler_function handler, int priority) noexcept;

        Breakpoint(volatile void *address, PCBreakpoint pc_breakpoint, handler_function handler, int priority) noexcept;

        Breakpoint(volatile void *address, PCBreakpoint pc_breakpoint, volatile void *spc, handler_function handler,
                   int priority) noexcept;

        Breakpoint(volatile void *address, handler_function handler) noexcept;

        Breakpoint(volatile void *address, volatile void *spc, handler_function handler) noexcept;

        Breakpoint(volatile void *address, PCBreakpoint pc_breakpoint, handler_function handler) noexcept;

        Breakpoint(volatile void *address, PCBreakpoint pc_breakpoint, volatile void *spc,
                   handler_function handler) noexcept;
    };

    class Singleton {
        friend bool ::ubi_c_handler(saved_regs *regs);

        Singleton() = default;

        bool active = false;

        std::vector<std::pair<volatile void *, handler_function> > computed_channel0_array;
        decltype(computed_channel0_array) computed_channel1_array;

        void recomputeArrays(volatile void *car0, std::uintptr_t camr0, volatile void *car1, std::uintptr_t camr1);

        void handleSingleTarget();

    public:
        Singleton(const Singleton &) = delete;

        Singleton &operator=(const Singleton &) = delete;

        ~Singleton();

        static Singleton &instance;

        std::vector<Breakpoint> handlers;

        void recomputeRegisters();

        void enable();

        void disable();
    };
}
