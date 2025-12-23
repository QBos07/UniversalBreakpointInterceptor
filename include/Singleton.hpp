#pragma once
#include <map>
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

        std::vector<std::pair<void *, handler_function>> computed_before_array;
        decltype(computed_before_array) computed_after_array;

        void recompute_arrays();

    public:
        Singleton(const Singleton&) = delete;
        Singleton& operator=(const Singleton&) = delete;
        ~Singleton();

        static Singleton& instance;

        // address to break on, expected spc, execution position, priority (negative to positive; equal unspecified), handler
        std::vector<std::tuple<void *, void *, PCBreakpoint, int, handler_function>> handlers;

        void recompute_registers();

        void enable();
        void disable();
    };
}
