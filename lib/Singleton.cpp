#include "Singleton.hpp"

#include <cstddef>
#include <cstdint>
//#include <cstdio>
#include <new>
#include <ranges>
#include <algorithm>
//#include <cinttypes>
#include <numeric>

#include "helper.hpp"
#include "ubc.h"

using namespace UBI;

alignas(Singleton) std::byte singleton_storage[sizeof(Singleton)];

Singleton &Singleton::instance = *new(singleton_storage) Singleton;

void Singleton::enable() {
    AS_STRUCT_SET(UBC_CBR0, {.id = ubc_cbr0_t::UBC_CBR0_ID_INSTRUCTION_FETCH_CYCLE, .ce = false});
    AS_STRUCT_SET(UBC_CBR1, {.id = ubc_cbr1_t::UBC_CBR1_ID_INSTRUCTION_FETCH_CYCLE, .ce = false});
    dummy_icbi();

    ubc_setDBR(&ubi_handler);
    AS_STRUCT_SET(UBC_CBCR, {.ubde = true});

    AS_STRUCT_SET(UBC_CCMFR, {false, false});

    recompute_registers();

    active = true;
}

void Singleton::disable() {
    if (!active) return;

    AS_STRUCT_SET(UBC_CBR0, {.id = ubc_cbr0_t::UBC_CBR0_ID_INSTRUCTION_FETCH_CYCLE, .ce = false});
    AS_STRUCT_SET(UBC_CBR1, {.id = ubc_cbr1_t::UBC_CBR1_ID_INSTRUCTION_FETCH_CYCLE, .ce = false});

    dummy_icbi();

    active = false;
}

Singleton::~Singleton() {
    disable();
}

void Singleton::recompute_arrays() {
    std::vector<std::size_t> indexes(handlers.size());
    std::iota(indexes.begin(), indexes.end(), 0z); // create a second indices array

    std::ranges::sort(indexes, std::less{}, [&handlers = handlers](const auto i) {
        const auto [addr, spc, bp, pri, handler] = handlers[i];
        return std::make_pair(spc, pri);
    });

    auto make_computed = [&handlers = handlers, &indexes](PCBreakpoint which) {
        return indexes |
               std::views::filter([&handlers = handlers, which](auto const i) {
                   // ignore elements not asked for
                   const auto [addr, spc, bp, pri, handler] = handlers[i];
                   return bp == which;
               }) | std::views::transform([&handlers = handlers](auto const i) {
                   // only save the needed parts
                   const auto [addr, spc, bp, pri, handler] = handlers[i];
                   return std::make_pair(spc, handler);
               });
    };

    // materialize
    computed_before_array = {std::from_range, make_computed(PCBreakpoint::BeforeExecution)};

    /*std::printf("Before: (%zu)\n", computed_before_array.size());
    for (auto const [addr, handler]: computed_before_array) {
        std::printf("%p - %p\n", addr, reinterpret_cast<void *>(handler));
    }*/

    computed_after_array = {std::from_range, make_computed(PCBreakpoint::AfterExecution)};

    /*std::printf("After: (%zu)\n", computed_after_array.size());
    for (auto const [addr, handler]: computed_after_array) {
        std::printf("%p - %p\n", addr, reinterpret_cast<void *>(handler));
    }*/
}

void Singleton::recompute_registers() {
    AS_STRUCT_SET(UBC_CBR0, {.id = ubc_cbr0_t::UBC_CBR0_ID_INSTRUCTION_FETCH_CYCLE, .ce = false});
    AS_STRUCT_SET(UBC_CBR1, {.id = ubc_cbr1_t::UBC_CBR1_ID_INSTRUCTION_FETCH_CYCLE, .ce = false});
    dummy_icbi();

    recompute_arrays();

    auto make_view = [&handlers = handlers](PCBreakpoint which) {
        return handlers
               | std::views::filter([which](auto const &val) {
                   const auto [addr, spc, bp, pri, handler] = val;
                   return bp == which;
               })
               | std::views::transform([](auto const &val) {
                   const auto [addr, spc, bp, pri, handler] = val;
                   return addr;
               });
    };

    auto get_non_uniform_bits = []<std::ranges::input_range R>(R view) -> std::uintptr_t requires std::convertible_to<
                std::ranges::range_value_t<R>, void *> {
        return view.empty()
                   ? 0
                   : std::transform_reduce(std::next(view.begin()), view.end(),
                                           std::uintptr_t{0},
                                           std::bit_or{},
                                           [f = reinterpret_cast<std::uintptr_t>(view.front())](void *p) {
                                               return f ^ reinterpret_cast<std::uintptr_t>(p);
                                           });
    };

    auto before_addresses = make_view(PCBreakpoint::BeforeExecution);
    auto after_addresses = make_view(PCBreakpoint::AfterExecution);

    *UBC_CAR0 = before_addresses.empty() ? nullptr : before_addresses.front();
    *UBC_CAMR0 = get_non_uniform_bits(before_addresses);

    *UBC_CAR1 = after_addresses.empty() ? nullptr : after_addresses.front();
    *UBC_CAMR1 = get_non_uniform_bits(after_addresses);

    //std::printf("Before CAR: %p CAMR: 0x%08" PRIx32 "\nAfter  CAR: %p CAMR: 0x%08" PRIx32 "\n", *UBC_CAR0, *UBC_CAMR0,
    //            *UBC_CAR1, *UBC_CAMR1);

    AS_STRUCT_SET(UBC_CRR0, {.pcb = ubc_crr_t::UBC_CCR_PCB_PC_BREAK_BEFORE_EXECUTION, .bie = true});
    AS_STRUCT_SET(UBC_CRR1, {.pcb = ubc_crr_t::UBC_CCR_PCB_PC_BREAK_AFTER_EXECUTIOM, .bie = true});

    AS_STRUCT_SET(UBC_CBR0, {.id = ubc_cbr0_t::UBC_CBR0_ID_INSTRUCTION_FETCH_CYCLE, .ce = true});
    AS_STRUCT_SET(UBC_CBR1, {.id = ubc_cbr1_t::UBC_CBR1_ID_INSTRUCTION_FETCH_CYCLE, .ce = true});

    dummy_icbi();
}
