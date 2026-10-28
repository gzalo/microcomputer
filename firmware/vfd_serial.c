#include "lcd.h"
#include "pins.h"
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/uart.h"

// CU20025SCPB-T20A command set. These are not HD44780 instructions.
#define VFD_CLEAR       0x0e
#define VFD_CURSOR_OFF  0x16
#define VFD_CURSOR_ON   0x14
#define VFD_CURSOR_BLINK 0x15
#define VFD_ESC         0x1b
#define VFD_POSITION    0x48

static void vfd_write(uint8_t byte) {
    uart_putc_raw(VFD_UART_PORT, byte);
}

void lcd_init(void) {
    uart_init(VFD_UART_PORT, VFD_UART_BAUD);
    uart_set_format(VFD_UART_PORT, 8, 1, UART_PARITY_EVEN);
    uart_set_hw_flow(VFD_UART_PORT, false, false);
    gpio_set_function(PIN_VFD_TX, GPIO_FUNC_UART);

    // The module initializes itself on power-up. Allow its supply to settle.
    sleep_ms(100);
    lcd_clear();
    lcd_display(true, false, false);
}

void lcd_command(uint8_t cmd) {
    // Preserve the two generic commands used by the LCD interface.
    if (cmd == LCD_CMD_CLEAR) {
        lcd_clear();
    } else if (cmd == LCD_CMD_HOME) {
        lcd_home();
    }
}

void lcd_data(uint8_t data) {
    vfd_write(data);
}

void lcd_clear(void) {
    vfd_write(VFD_CLEAR);
    sleep_ms(2);
}

void lcd_home(void) {
    lcd_set_cursor(0, 0);
}

void lcd_set_cursor(uint8_t col, uint8_t row) {
    if (col >= 20 || row >= 2) return;
    vfd_write(VFD_ESC);
    vfd_write(VFD_POSITION);
    vfd_write(row * 20 + col);
}

void lcd_print(const char *str) {
    while (*str) lcd_data((uint8_t)*str++);
}

void lcd_putchar(char c) {
    lcd_data((uint8_t)c);
}

void lcd_print_hex8(uint8_t value) {
    static const char hex[] = "0123456789ABCDEF";
    lcd_data(hex[value >> 4]);
    lcd_data(hex[value & 0x0f]);
}

void lcd_print_hex16(uint16_t value) {
    lcd_print_hex8((uint8_t)(value >> 8));
    lcd_print_hex8((uint8_t)value);
}

void lcd_display(bool on, bool cursor, bool blink) {
    // This module has no simple display-off command. All current callers use on=true.
    (void)on;
    vfd_write(cursor ? (blink ? VFD_CURSOR_BLINK : VFD_CURSOR_ON) : VFD_CURSOR_OFF);
}
