#pragma once

#ifdef __cplusplus
extern "C" {
#else
#if __STDC_VERSION__ < 202311L
#include <stdbool.h>
#endif
#endif

#include <stdint.h>
#include <stddef.h>

typedef struct HandlerContext HandlerContext;
typedef struct Register Register;

typedef void (*handler_function)(HandlerContext *);

typedef enum PCBreakpoint {
    BP_PC_BeforeExecution,
    BP_PC_AfterExecution
} PCBreakpoint;

typedef struct Breakpoint {
    volatile void *address;
    PCBreakpoint pc_breakpoint;
    volatile void *spc;
    handler_function handler;
    //negative to positive; equal unspecified
    int priority;
} Breakpoint;

void ubi_enable();

void ubi_disable();

void ubi_deconstruct();

void ubi_recompute();

size_t ubi_handlers_count();

Breakpoint ubi_handlers_get(size_t index);

void ubi_handlers_add_breakpoint(Breakpoint breakpoint);

void ubi_handlers_add(volatile void *address, PCBreakpoint pc_breakpoint, handler_function handler, int priority);

void ubi_handlers_add_with_spc(volatile void *address, PCBreakpoint pc_breakpoint, volatile void *spc,
                               handler_function handler, int priority);

void ubi_handlers_remove(ptrdiff_t index);

void ubi_handlers_clear();

void handler_context_allow_nested_interrupts(HandlerContext *ctx);

void handler_context_disallow_nested_interrupts(HandlerContext *ctx);

void register_free(const Register *reg);

// 0-15 changed
Register *handler_context_get_register(HandlerContext *ctx, size_t number) __attribute__((malloc(register_free)));

// bank: 0-1 number: 0-7 underlying is const if original otherwise changes
Register *handler_context_get_banked_register(HandlerContext *ctx, size_t bank, size_t number, bool original)
__attribute__((malloc(register_free)));

// 0-7 for 8-15 underlying is const if original otherwise changes
Register *handler_context_get_unbanked_register(HandlerContext *ctx, size_t number, bool original)
__attribute__((malloc(register_free)));

// underlying is const if original
Register *handler_context_get_sr_register(HandlerContext *ctx, bool original) __attribute__((malloc(register_free)));

// underlying is const if original
Register *handler_context_get_pc_register(HandlerContext *ctx, bool original) __attribute__((malloc(register_free)));

// underlying is const if original
Register *handler_context_get_pr_register(HandlerContext *ctx, bool original) __attribute__((malloc(register_free)));

// underlying is const if original
Register *handler_context_get_macl_register(HandlerContext *ctx, bool original) __attribute__((malloc(register_free)));

// underlying is const if original
Register *handler_context_get_mach_register(HandlerContext *ctx, bool original) __attribute__((malloc(register_free)));

void *register_as_ptr(const Register *reg);

void **register_as_ptr_ptr(const Register *reg);

bool register_as_bool(const Register *reg);

char register_as_char(const Register *reg);

short register_as_short(const Register *reg);

int register_as_int(const Register *reg);

long register_as_long(const Register *reg);

unsigned short register_as_ushort(const Register *reg);

unsigned int register_as_uint(const Register *reg);

unsigned long register_as_ulong(const Register *reg);

float register_as_float(const Register *reg);

uint8_t register_as_uint8(const Register *reg);

uint16_t register_as_uint16(const Register *reg);

uint32_t register_as_uint32(const Register *reg);

int8_t register_as_int8(const Register *reg);

int16_t register_as_int16(const Register *reg);

int32_t register_as_int32(const Register *reg);

size_t register_as_size(const Register *reg);

uintptr_t register_as_uintptr(const Register *reg);

void register_save_ptr(const Register *reg, volatile const void *value);

void register_save_ptr_ptr(const Register *reg, volatile const void *volatile const *value);

void register_save_bool(const Register *reg, bool value);

void register_save_char(const Register *reg, char value);

void register_save_short(const Register *reg, short value);

void register_save_int(const Register *reg, int value);

void register_save_long(const Register *reg, long value);

void register_save_ushort(const Register *reg, unsigned short value);

void register_save_uint(const Register *reg, unsigned int value);

void register_save_ulong(const Register *reg, unsigned long value);

void register_save_float(const Register *reg, float value);

void register_save_uint8(const Register *reg, uint8_t value);

void register_save_uint16(const Register *reg, uint16_t value);

void register_save_uint32(const Register *reg, uint32_t value);

void register_save_int8(const Register *reg, int8_t value);

void register_save_int16(const Register *reg, int16_t value);

void register_save_int32(const Register *reg, int32_t value);

void register_save_size(const Register *reg, size_t value);

void register_save_uintptr(const Register *reg, uintptr_t value);

#ifdef __cplusplus
}
#endif
