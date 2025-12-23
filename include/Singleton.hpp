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

    class Singleton {
        friend bool ::ubi_c_handler(saved_regs *regs);

        Singleton() = default;

        bool active = false;

        std::vector<std::pair<volatile void *, handler_function>> computed_channel0_array;
        decltype(computed_channel0_array) computed_channel1_array;

        void recompute_arrays(volatile void *car0, std::uintptr_t camr0, volatile void *car1, std::uintptr_t camr1);
        void handle_single_target();

    public:
        Singleton(const Singleton&) = delete;
        Singleton& operator=(const Singleton&) = delete;
        ~Singleton();

        static Singleton& instance;

        // address to break on, expected spc, execution position, priority (negative to positive; equal unspecified), handler
        std::vector<std::tuple<volatile void *, volatile void *, PCBreakpoint, int, handler_function>> handlers;

        void recompute_registers();

        void enable();
        void disable();
    };
}
