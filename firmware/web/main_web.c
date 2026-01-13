#include <emscripten.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

// External LCD/LED state (defined in lcd_web.c and shift_register_web.c)
extern char lcd_buffer[2][21];
extern uint8_t lcd_cursor_col;
extern uint8_t lcd_cursor_row;
extern bool lcd_cursor_on;
extern bool lcd_display_on;
extern uint16_t led_pattern;

// Stub for pico/stdlib.h time functions used by microcomputer.c
static uint32_t web_time_ms = 0;

uint32_t to_ms_since_boot(uint64_t t) {
    (void)t;
    return web_time_ms;
}

uint64_t get_absolute_time(void) {
    return (uint64_t)web_time_ms;
}

void sleep_ms(uint32_t ms) {
    (void)ms; // No-op on web
}

// Include emulator headers
#include "lcd.h"
#include "shift_register.h"
#include "../cpu8080.h"
#include "../memory.h"
#include "../disasm.h"
#include "../microcomputer.h"

static emulator_t emu;

// Export functions for JavaScript
EMSCRIPTEN_KEEPALIVE
void emu_init(void) {
    memory_init();
    emulator_init(&emu);
    lcd_init();
}

EMSCRIPTEN_KEEPALIVE
void emu_set_time(uint32_t ms) {
    web_time_ms = ms;
}

EMSCRIPTEN_KEEPALIVE
void emu_update(uint16_t switches, uint16_t buttons) {
    emulator_update(&emu, switches, buttons);
}

EMSCRIPTEN_KEEPALIVE
const char* emu_get_lcd_line(int row) {
    if (row < 0 || row > 1) return "";
    return lcd_buffer[row];
}

EMSCRIPTEN_KEEPALIVE
uint8_t emu_get_lcd_cursor_col(void) {
    return lcd_cursor_col;
}

EMSCRIPTEN_KEEPALIVE
uint8_t emu_get_lcd_cursor_row(void) {
    return lcd_cursor_row;
}

EMSCRIPTEN_KEEPALIVE
int emu_get_lcd_cursor_on(void) {
    return lcd_cursor_on ? 1 : 0;
}

EMSCRIPTEN_KEEPALIVE
uint16_t emu_get_led_pattern(void) {
    return led_pattern;
}

EMSCRIPTEN_KEEPALIVE
uint16_t emu_get_pc(void) {
    return emu.cpu.pc;
}

EMSCRIPTEN_KEEPALIVE
uint8_t emu_get_reg_a(void) { return emu.cpu.a; }

EMSCRIPTEN_KEEPALIVE
uint8_t emu_get_reg_f(void) { return emu.cpu.f; }

EMSCRIPTEN_KEEPALIVE
uint8_t emu_get_reg_b(void) { return emu.cpu.b; }

EMSCRIPTEN_KEEPALIVE
uint8_t emu_get_reg_c(void) { return emu.cpu.c; }

EMSCRIPTEN_KEEPALIVE
uint8_t emu_get_reg_d(void) { return emu.cpu.d; }

EMSCRIPTEN_KEEPALIVE
uint8_t emu_get_reg_e(void) { return emu.cpu.e; }

EMSCRIPTEN_KEEPALIVE
uint8_t emu_get_reg_h(void) { return emu.cpu.h; }

EMSCRIPTEN_KEEPALIVE
uint8_t emu_get_reg_l(void) { return emu.cpu.l; }

EMSCRIPTEN_KEEPALIVE
uint16_t emu_get_sp(void) { return emu.cpu.sp; }

EMSCRIPTEN_KEEPALIVE
int emu_is_halted(void) { return emu.cpu.halted ? 1 : 0; }

EMSCRIPTEN_KEEPALIVE
int emu_get_run_mode(void) { return (int)emu.run_mode; }

EMSCRIPTEN_KEEPALIVE
uint8_t emu_read_memory(uint16_t addr) {
    return memory_read(addr);
}

EMSCRIPTEN_KEEPALIVE
void emu_write_memory(uint16_t addr, uint8_t data) {
    memory_write(addr, data);
}

EMSCRIPTEN_KEEPALIVE
int emu_disasm(uint16_t addr, char *buffer, int size) {
    return disasm_instruction(addr, buffer, size);
}

int main(void) {
    emu_init();
    return 0;
}
