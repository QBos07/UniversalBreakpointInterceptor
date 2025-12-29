#pragma once

#include <utility>
#include <type_traits>

#include "ubi.h"
#include "Singleton.hpp"

static_assert(std::is_same_v<decltype(UBI::Singleton::handlers)::size_type, size_t>);
static_assert(std::is_same_v<decltype(UBI::Singleton::handlers)::const_iterator::difference_type, ptrdiff_t>);

constexpr UBI::PCBreakpoint enum_map(const PCBreakpoint bp) {
    switch (bp) {
        case BP_PC_BeforeExecution: return UBI::PCBreakpoint::BeforeExecution;
        case BP_PC_AfterExecution: return UBI::PCBreakpoint::AfterExecution;
    }
    std::unreachable();
}

constexpr PCBreakpoint enum_map(const UBI::PCBreakpoint bp) {
    switch (bp) {
        case UBI::PCBreakpoint::BeforeExecution: return BP_PC_BeforeExecution;
        case UBI::PCBreakpoint::AfterExecution: return BP_PC_AfterExecution;
    }
    std::unreachable();
}

extern "C" {
    struct Register {
    private:
        const UBI::Register &reg;
        bool constness;
    public:
        explicit Register(UBI::Register &reg) noexcept;
        explicit Register(const UBI::Register &reg) noexcept;
        [[nodiscard]] const UBI::Register &getConst() const noexcept;
        [[nodiscard]] UBI::Register &get() const;
    };
}
