#include "ubi_private.hpp"

auto &S = UBI::Singleton::instance;

Register::Register(UBI::Register &reg) noexcept : reg(reg), constness(false) {
}

Register::Register(const UBI::Register &reg) noexcept : reg(reg), constness(true) {
}

const UBI::Register &Register::getConst() const noexcept { return reg; }

UBI::Register &Register::get() const {
    if (constness)
        throw std::bad_cast();
    return const_cast<UBI::Register &>(reg);
}

extern "C" {
void ubi_enable() {
    S.enable();
}

void ubi_disable() {
    S.disable();
}

void ubi_recompute() {
    S.recomputeRegisters();
}

void ubi_deconstruct() {
    S.~Singleton();
}

size_t ubi_handlers_count() {
    return S.handlers.size();
}

Breakpoint ubi_handlers_get(const size_t index) {
    const auto &real = S.handlers[index];
    return {
        .address = real.address, .pc_breakpoint = enum_map(real.pc_breakpoint), .spc = real.spc,
        .handler = reinterpret_cast<handler_function>(reinterpret_cast<void (*)()>(real.handler)),
        .priority = real.priority
    };
}

void ubi_handlers_add_breakpoint(const Breakpoint breakpoint) {
    ubi_handlers_add_with_spc(breakpoint.address, breakpoint.pc_breakpoint, breakpoint.spc, breakpoint.handler,
                              breakpoint.priority);
}

void ubi_handlers_add(volatile void *const address, const PCBreakpoint pc_breakpoint, const handler_function handler,
                      const int priority) {
    S.handlers.emplace_back(address, enum_map(pc_breakpoint),
                            reinterpret_cast<UBI::handler_function>(reinterpret_cast<void (*)()>(handler)), priority);
}

void ubi_handlers_add_with_spc(volatile void *const address, const PCBreakpoint pc_breakpoint, volatile void *const spc,
                               const handler_function handler, const int priority) {
    S.handlers.emplace_back(address, enum_map(pc_breakpoint), spc,
                            reinterpret_cast<UBI::handler_function>(reinterpret_cast<void (*)()>(handler)), priority);
}

void ubi_handlers_remove(const ptrdiff_t index) {
    S.handlers.erase(S.handlers.begin() + index);
}

void ubi_handlers_clear() {
    S.handlers.clear();
}

void handler_context_allow_nested_interrupts(HandlerContext *const ctx) {
    reinterpret_cast<UBI::HandlerContext *>(ctx)->allowNestedInterrupts();
}

void handler_context_disallow_nested_interrupts(HandlerContext *const ctx) {
    reinterpret_cast<UBI::HandlerContext *>(ctx)->disallowNestedInterrupts();
}

Register *handler_context_get_register(HandlerContext *const ctx, const size_t number) {
    return new Register(reinterpret_cast<UBI::HandlerContext *>(ctx)->R[number]);
}

Register *handler_context_get_banked_register(HandlerContext *const ctx, const size_t bank, const size_t number,
                                              const bool original) {
    auto &context = *reinterpret_cast<UBI::HandlerContext *>(ctx);
    if (original)
        return new Register(context.original_registers.banked[bank][number]);
    return new Register(context.registers.banked[bank][number]);
}

Register *handler_context_get_unbanked_register(HandlerContext *const ctx, const size_t number, const bool original) {
    auto &context = *reinterpret_cast<UBI::HandlerContext *>(ctx);
    if (original)
        return new Register(context.original_registers.unbanked[number]);
    return new Register(context.registers.unbanked[number]);
}

Register *handler_context_get_sr_register(HandlerContext *const ctx, const bool original) {
    auto &context = *reinterpret_cast<UBI::HandlerContext *>(ctx);
    if (original)
        return new Register(context.original_registers.SR);
    return new Register(context.registers.SR);
}

Register *handler_context_get_pc_register(HandlerContext *const ctx, const bool original) {
    auto &context = *reinterpret_cast<UBI::HandlerContext *>(ctx);
    if (original)
        return new Register(context.original_registers.PC);
    return new Register(context.registers.PC);
}

Register *handler_context_get_pr_register(HandlerContext *const ctx, const bool original) {
    auto &context = *reinterpret_cast<UBI::HandlerContext *>(ctx);
    if (original)
        return new Register(context.original_registers.PR);
    return new Register(context.registers.PR);
}

Register *handler_context_get_macl_register(HandlerContext *const ctx, const bool original) {
    auto &context = *reinterpret_cast<UBI::HandlerContext *>(ctx);
    if (original)
        return new Register(context.original_registers.MACL);
    return new Register(context.registers.MACL);
}

Register *handler_context_get_mach_register(HandlerContext *const ctx, const bool original) {
    auto &context = *reinterpret_cast<UBI::HandlerContext *>(ctx);
    if (original)
        return new Register(context.original_registers.MACH);
    return new Register(context.registers.MACH);
}

void *register_as_ptr(const Register *const reg) { return reg->getConst().asCopy<void *>(); }
void **register_as_ptr_ptr(const Register *const reg) { return reg->getConst().asCopy<void **>(); }
bool register_as_bool(const Register *const reg) { return reg->getConst().asCopy<bool>(); }
char register_as_char(const Register *const reg) { return reg->getConst().asCopy<char>(); }
short register_as_short(const Register *const reg) { return reg->getConst().asCopy<short>(); }
int register_as_int(const Register *const reg) { return reg->getConst().asCopy<int>(); }
long register_as_long(const Register *const reg) { return reg->getConst().asCopy<long>(); }
unsigned short register_as_ushort(const Register *const reg) { return reg->getConst().asCopy<unsigned short>(); }
unsigned int register_as_uint(const Register *const reg) { return reg->getConst().asCopy<unsigned int>(); }
unsigned long register_as_ulong(const Register *const reg) { return reg->getConst().asCopy<unsigned long>(); }
float register_as_float(const Register *const reg) { return reg->getConst().asCopy<float>(); }
uint8_t register_as_uint8(const Register *const reg) { return reg->getConst().asCopy<uint8_t>(); }
uint16_t register_as_uint16(const Register *const reg) { return reg->getConst().asCopy<uint16_t>(); }
uint32_t register_as_uint32(const Register *const reg) { return reg->getConst().asCopy<uint32_t>(); }
int8_t register_as_int8(const Register *const reg) { return reg->getConst().asCopy<int8_t>(); }
int16_t register_as_int16(const Register *const reg) { return reg->getConst().asCopy<int16_t>(); }
int32_t register_as_int32(const Register *const reg) { return reg->getConst().asCopy<int32_t>(); }
size_t register_as_size(const Register *const reg) { return reg->getConst().asCopy<size_t>(); }
uintptr_t register_as_uintptr(const Register *const reg) { return reg->getConst().asCopy<uintptr_t>(); }

void register_save_ptr(const Register *const reg, volatile const void *const value) { reg->get() = value; }

void register_save_ptr_ptr(const Register *const reg, volatile const void *volatile const *const value) {
    reg->get() = value;
}

void register_save_bool(const Register *const reg, const bool value) { reg->get() = value; }
void register_save_char(const Register *const reg, const char value) { reg->get() = value; }
void register_save_short(const Register *const reg, const short value) { reg->get() = value; }
void register_save_int(const Register *const reg, const int value) { reg->get() = value; }
void register_save_long(const Register *const reg, const long value) { reg->get() = value; }
void register_save_ushort(const Register *const reg, const unsigned short value) { reg->get() = value; }
void register_save_uint(const Register *const reg, const unsigned int value) { reg->get() = value; }
void register_save_ulong(const Register *const reg, const unsigned long value) { reg->get() = value; }
void register_save_float(const Register *const reg, const float value) { reg->get() = value; }
void register_save_uint8(const Register *const reg, const uint8_t value) { reg->get() = value; }
void register_save_uint16(const Register *const reg, const uint16_t value) { reg->get() = value; }
void register_save_uint32(const Register *const reg, const uint32_t value) { reg->get() = value; }
void register_save_int8(const Register *const reg, const int8_t value) { reg->get() = value; }
void register_save_int16(const Register *const reg, const int16_t value) { reg->get() = value; }
void register_save_int32(const Register *const reg, const int32_t value) { reg->get() = value; }
void register_save_size(const Register *const reg, const size_t value) { reg->get() = value; }
void register_save_uintptr(const Register *const reg, const uintptr_t value) { reg->get() = value; }

void register_free(const Register *const reg) {
    delete reg;
}
}
