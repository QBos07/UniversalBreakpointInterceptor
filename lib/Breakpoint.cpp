#include "Singleton.hpp"

using namespace UBI;

Breakpoint::Breakpoint(volatile void *const address, const handler_function handler,
                       const int priority) noexcept : address(address), handler(handler), priority(priority) {
}

Breakpoint::Breakpoint(volatile void *const address, volatile void *const spc, const handler_function handler,
                       const int priority) noexcept : address(address), spc(spc), handler(handler), priority(priority) {
}

Breakpoint::Breakpoint(volatile void *const address, const PCBreakpoint pc_breakpoint, const handler_function handler,
                       const int priority) noexcept : address(address), pc_breakpoint(pc_breakpoint), handler(handler),
                                                      priority(priority) {
}

Breakpoint::Breakpoint(volatile void *const address, const PCBreakpoint pc_breakpoint, volatile void *const spc,
                       const handler_function handler, const int priority) noexcept : address(address),
    pc_breakpoint(pc_breakpoint), spc(spc), handler(handler), priority(priority) {
}

Breakpoint::Breakpoint(volatile void *const address, const handler_function handler) noexcept : address(address),
    handler(handler) {
}

Breakpoint::Breakpoint(volatile void *const address, volatile void *const spc,
                       const handler_function handler) noexcept : address(address), spc(spc), handler(handler) {
}

Breakpoint::Breakpoint(volatile void *const address, const PCBreakpoint pc_breakpoint,
                       const handler_function handler) noexcept : address(address), pc_breakpoint(pc_breakpoint),
                                                                  handler(handler) {
}

Breakpoint::Breakpoint(volatile void *const address, const PCBreakpoint pc_breakpoint, volatile void *const spc,
                       const handler_function handler) noexcept : address(address), pc_breakpoint(pc_breakpoint),
                                                                  spc(spc), handler(handler) {
}
