#include <appdef.h>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <sdk/os/debug.h>
#include <sdk/calc/calc.h>
#include <sdk/os/lcd.h>
#include "Singleton.hpp"

APP_NAME("UBI Test")

APP_AUTHOR("QBos07")

APP_DESCRIPTION("Example usage of the Universal Breakpoint Interceptor library")

APP_VERSION("1.0.0")

using S = UBI::Singleton;

[[gnu::noinline]] void fun1() {
    std::printf("1!\n");
    __asm__ volatile ("");
}

[[gnu::noinline]] void fun2() {
    std::printf("2!\n");
    __asm__ volatile ("");
}

[[gnu::noinline]] void fun3() {
    std::printf("3!\n");
    __asm__ volatile ("");
}

void handler_after(UBI::HandlerContext &ctx) {
    ctx.allowNestedInterrupts();
    fun3();
    std::printf("Hello");
}

void handler2(UBI::HandlerContext &) {
    std::printf("Nested!");
}

void other_handler(UBI::HandlerContext &) {
    std::printf(" ");
}

void handler_before(UBI::HandlerContext &ctx) {
    std::printf("World\n");
    ctx.registers.PC = ctx.registers.PR.as<void *>();
}

[[gnu::used]] int main() {
    auto stack = new std::byte[0x1000]; //optional
    ubi_debug_stack = stack + 0x1000 - sizeof(int);
    /*S::instance.handlers.emplace_back(reinterpret_cast<void *>(fun1), handler_after);
    S::instance.handlers.emplace_back(reinterpret_cast<void *>(fun2), handler_after);
    S::instance.handlers.emplace_back(reinterpret_cast<void *>(fun1), other_handler, 1);
    S::instance.handlers.emplace_back(reinterpret_cast<void *>(fun2), other_handler, 1);
    S::instance.handlers.emplace_back(reinterpret_cast<void *>(fun3), handler2);
    S::instance.handlers.emplace_back(reinterpret_cast<void *>(fun3), other_handler, 1);*/
    S::instance.handlers.emplace_back(reinterpret_cast<void *>(fun3), UBI::PCBreakpoint::BeforeExecution,
                                      handler_before);
    S::instance.enable();
    fun1();
    fun2();
    fun3();
    S::instance.~Singleton();
    delete[] stack;
    std::fflush(stdout);
    Debug_WaitKey();
    return 0;
}

// A backup pointer for the original vram content
std::remove_pointer_t<decltype(vram)> (*vram_bak)[width * height];

// This function is called when your app starts
void calcInit() {
    // Allocate memory on the heap to store the screen backup
    vram_bak = new std::remove_pointer_t<decltype(vram_bak)>[1];
    // Copy the current screen content to our backup buffer
    std::memcpy(vram_bak, vram, sizeof(*vram_bak));
    // Clear the screen for the app
    std::memset(vram, 0, sizeof(*vram_bak));
}

// This function is called when your app exits
void calcExit() {
    // Copy the backed-up screen content back to vram
    std::memcpy(vram, vram_bak, sizeof(*vram_bak));
    // Refresh the LCD to show the restored content
    LCD_Refresh();
    // Free the memory we allocated for the backup
    delete[] vram_bak;
}
