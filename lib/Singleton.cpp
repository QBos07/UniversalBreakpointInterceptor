#include "Singleton.hpp"

#include <cstddef>
#include <bit>
#include <new>
#include <ranges>
#include <algorithm>
//#include <cinttypes>
#include <numeric>
#include <set>

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

    recomputeRegisters();

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

void Singleton::recomputeArrays(volatile void *car0, std::uintptr_t camr0, volatile void *car1, std::uintptr_t camr1) {
    std::vector<std::size_t> indexes(handlers.size());
    std::iota(indexes.begin(), indexes.end(), 0zu); // create a second indices array

    std::ranges::sort(indexes, std::less{}, [&handlers = handlers](const auto i) {
        const auto [addr, bp, spc, handler, pri] = handlers[i];
        return std::make_pair(spc, pri);
    });

    const auto makeFiltered = [&handlers = handlers, &indexes](ubc_car_t car, ubc_camr_t camr) {
        return indexes |
               std::views::filter([&handlers = handlers, car, camr](auto const i) {
                   // ignore elements not asked for
                   const auto [addr, bp, spc, handler, pri] = handlers[i];
                   return (reinterpret_cast<std::uintptr_t>(addr) & ~camr) == (
                              reinterpret_cast<std::uintptr_t>(car) & ~camr);
               }) | std::views::transform([&handlers = handlers](auto const i) {
                   const auto [addr, bp, spc, handler, pri] = handlers[i];
                   return std::make_pair(spc, handler);
               });;
    };

    // materialize
    computed_channel0_array = {std::from_range, makeFiltered(car0, camr0)};
    computed_channel1_array = {std::from_range, makeFiltered(car1, camr1)};

    std::printf("CH0: (%zu)\n", computed_channel0_array.size());
    for (const auto &[spc, handler]: computed_channel0_array) {
        std::printf(" %p - %p\n", spc, reinterpret_cast<void *>(handler));
    }
    std::printf("CH1: (%zu)\n", computed_channel0_array.size());
    for (const auto &[spc, handler]: computed_channel1_array) {
        std::printf(" %p - %p\n", spc, reinterpret_cast<void *>(handler));
    }
}

template<std::ranges::input_range R>
    requires std::convertible_to<std::ranges::range_value_t<R>, volatile void *>
static std::pair<std::pair<ubc_car_t, ubc_camr_t>, std::pair<ubc_car_t, ubc_camr_t> > computeRegistersBoth(
    const R &before_addresses, const R &after_addresses) {
    const auto getNonUniformBits = []<std::ranges::input_range V>(V view) -> std::uintptr_t {
        return std::transform_reduce(std::next(view.begin()), view.end(),
                                     std::uintptr_t{0},
                                     std::bit_or{},
                                     [f = reinterpret_cast<std::uintptr_t>(*view.begin())](volatile void *p) {
                                         return f ^ reinterpret_cast<std::uintptr_t>(p);
                                     });
    };

    AS_STRUCT_SET(UBC_CRR0, {.pcb = ubc_crr_t::UBC_CCR_PCB_PC_BREAK_BEFORE_EXECUTION, .bie = true});
    AS_STRUCT_SET(UBC_CRR1, {.pcb = ubc_crr_t::UBC_CCR_PCB_PC_BREAK_AFTER_EXECUTIOM, .bie = true});

    return std::make_pair(std::make_pair(*before_addresses.begin(), getNonUniformBits(before_addresses)),
                          std::make_pair(*after_addresses.begin(), getNonUniformBits(after_addresses)));
}

template<std::ranges::input_range R>
    requires std::convertible_to<std::ranges::range_value_t<R>, volatile void *>
static std::pair<std::pair<ubc_car_t, ubc_camr_t>, std::pair<ubc_car_t, ubc_camr_t> > computeRegistersOnlyOne(
    const R &addresses) {
    auto uintptr_addresses = std::vector{
        std::from_range,
        addresses | std::views::transform([](const auto val) { return reinterpret_cast<std::uintptr_t>(val); })
    };

    const auto FULL = 1zu << uintptr_addresses.size();

    std::vector<ubc_camr_t> precomputed_and(FULL);
    std::vector<ubc_camr_t> precomputed_or(FULL);

    precomputed_or[0] = 0;
    precomputed_and[0] = ~precomputed_or[0];

    for (auto mask = decltype(FULL){1}; mask < FULL; ++mask) {
        auto bit = std::countr_zero(mask);
        auto prev = mask & ~(decltype(mask){1} << bit);

        precomputed_and[mask] = precomputed_and[prev] & uintptr_addresses[bit];
        precomputed_or[mask] = precomputed_or[prev] | uintptr_addresses[bit];
    }

    auto view = std::views::iota(decltype(FULL){1}, FULL) | std::views::transform(
                    [FULL, &precomputed_and, &precomputed_or](const auto mask) {
                        auto comp = (FULL - 1) ^ mask;

                        auto camr0 = precomputed_and[mask] ^ precomputed_or[mask];
                        auto camr1 = precomputed_and[comp] ^ precomputed_or[comp];

                        auto car0 = reinterpret_cast<ubc_car_t>(precomputed_and[mask]);
                        auto car1 = reinterpret_cast<ubc_car_t>(precomputed_and[comp]);

                        auto cost = std::popcount(camr0) + std::popcount(camr1);

                        return std::make_pair(cost, std::make_pair(std::make_pair(car0, camr0),
                                                                   std::make_pair(car1, camr1)));
                    });

    auto min = std::ranges::min(view, {}, [](const auto &val) { return val.first; });

    return min.second;
}

void Singleton::handleSingleTarget() {
    const auto [addr, bp, spc, handler, pri] = handlers.front();
    *UBC_CAR0 = addr;
    *UBC_CAMR0 = 0;
    computed_channel0_array = {
        std::from_range, handlers | std::views::transform([spc](const auto &val) {
            const auto [addr, bp, spc_val, handler, pri] = val;
            return std::make_pair(spc, handler);
        })
    };
    computed_channel1_array = {};
    switch (bp) {
        case PCBreakpoint::BeforeExecution:
            AS_STRUCT_SET(UBC_CRR0, {.pcb = ubc_crr_t::UBC_CCR_PCB_PC_BREAK_BEFORE_EXECUTION, .bie = true});
            break;
        case PCBreakpoint::AfterExecution:
            AS_STRUCT_SET(UBC_CRR0, {.pcb = ubc_crr_t::UBC_CCR_PCB_PC_BREAK_AFTER_EXECUTIOM, .bie = true});
            break;
    }

    AS_STRUCT_SET(UBC_CBR0, {.id = ubc_cbr0_t::UBC_CBR0_ID_INSTRUCTION_FETCH_CYCLE, .ce = true});

    dummy_icbi();
}

void Singleton::recomputeRegisters() {
    AS_STRUCT_SET(UBC_CBR0, {.id = ubc_cbr0_t::UBC_CBR0_ID_INSTRUCTION_FETCH_CYCLE, .ce = false});
    AS_STRUCT_SET(UBC_CBR1, {.id = ubc_cbr1_t::UBC_CBR1_ID_INSTRUCTION_FETCH_CYCLE, .ce = false});
    dummy_icbi();

    if (handlers.empty()) return;

    const auto makeView = [&handlers = handlers](PCBreakpoint which) {
        return handlers
               | std::views::filter([which](auto const &val) {
                   const auto [addr, bp, spc_val, handler, pri] = val;
                   return bp == which;
               })
               | std::views::transform([](auto const &val) {
                   const auto [addr, bp, spc_val, handler, pri] = val;
                   return addr;
               });
    };

    auto before_addresses = makeView(PCBreakpoint::BeforeExecution) | std::ranges::to<std::set>();
    auto after_addresses = makeView(PCBreakpoint::AfterExecution) | std::ranges::to<std::set>();

    if (before_addresses.size() + after_addresses.size() == 1) {
        handleSingleTarget();
        return;
    }

    const auto [channel0, channel1] = [&before_addresses, &after_addresses] {
        if (!before_addresses.empty() && after_addresses.empty()) {
            AS_STRUCT_SET(UBC_CRR0, {.pcb = ubc_crr_t::UBC_CCR_PCB_PC_BREAK_BEFORE_EXECUTION, .bie = true});
            AS_STRUCT_SET(UBC_CRR1, {.pcb = ubc_crr_t::UBC_CCR_PCB_PC_BREAK_BEFORE_EXECUTION, .bie = true});

            return computeRegistersOnlyOne(before_addresses);
        }

        if (before_addresses.empty() && !after_addresses.empty()) {
            AS_STRUCT_SET(UBC_CRR0, {.pcb = ubc_crr_t::UBC_CCR_PCB_PC_BREAK_AFTER_EXECUTIOM, .bie = true});
            AS_STRUCT_SET(UBC_CRR1, {.pcb = ubc_crr_t::UBC_CCR_PCB_PC_BREAK_AFTER_EXECUTIOM, .bie = true});

            return computeRegistersOnlyOne(after_addresses);
        }

        return computeRegistersBoth(before_addresses, after_addresses);
    }();

    const auto [car0, camr0] = channel0;
    const auto [car1, camr1] = channel1;

    //std::printf("CAR0: %p, CAMR0: 0x%08" PRIx32 "\nCAR1: %p, CAMR1: 0x%08" PRIx32 "\n", car0, camr0, car1, camr1);
    recomputeArrays(car0, camr0, car1, camr1);

    *UBC_CAR0 = car0;
    *UBC_CAR1 = car1;
    *UBC_CAMR0 = camr0;
    *UBC_CAMR1 = camr1;

    AS_STRUCT_SET(UBC_CBR0, {.id = ubc_cbr0_t::UBC_CBR0_ID_INSTRUCTION_FETCH_CYCLE, .ce = true});
    AS_STRUCT_SET(UBC_CBR1, {.id = ubc_cbr1_t::UBC_CBR1_ID_INSTRUCTION_FETCH_CYCLE, .ce = true});

    dummy_icbi();
}
