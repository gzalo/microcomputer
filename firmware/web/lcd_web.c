// Web LCD implementation - buffer-based
#include "lcd.h"
#include <string.h>

// LCD buffer and cursor state
char lcd_buffer[2][21];
uint8_t lcd_cursor_col = 0;
uint8_t lcd_cursor_row = 0;
bool lcd_cursor_on = false;
bool lcd_display_on = true;

void lcd_init(void) {
    memset(lcd_buffer[0], ' ', 20);
    memset(lcd_buffer[1], ' ', 20);
    lcd_buffer[0][20] = '\0';
    lcd_buffer[1][20] = '\0';
    lcd_cursor_col = 0;
    lcd_cursor_row = 0;
    lcd_cursor_on = false;
    lcd_display_on = true;
}

void lcd_clear(void) {
    memset(lcd_buffer[0], ' ', 20);
    memset(lcd_buffer[1], ' ', 20);
    lcd_cursor_col = 0;
    lcd_cursor_row = 0;
}

void lcd_set_cursor(uint8_t col, uint8_t row) {
    lcd_cursor_col = col < 20 ? col : 19;
    lcd_cursor_row = row < 2 ? row : 1;
}

void lcd_print(const char *str) {
    while (*str && lcd_cursor_col < 20) {
        lcd_buffer[lcd_cursor_row][lcd_cursor_col++] = *str++;
    }
}

void lcd_putchar(char c) {
    if (lcd_cursor_col < 20) {
        lcd_buffer[lcd_cursor_row][lcd_cursor_col++] = c;
    }
}

void lcd_print_hex8(uint8_t value) {
    static const char hex[] = "0123456789ABCDEF";
    lcd_putchar(hex[(value >> 4) & 0x0F]);
    lcd_putchar(hex[value & 0x0F]);
}

void lcd_print_hex16(uint16_t value) {
    lcd_print_hex8((value >> 8) & 0xFF);
    lcd_print_hex8(value & 0xFF);
}

void lcd_display(bool on, bool cursor, bool blink) {
    lcd_display_on = on;
    lcd_cursor_on = cursor;
    (void)blink;
}

void lcd_command(uint8_t cmd) { (void)cmd; }
void lcd_data(uint8_t data) { (void)data; }
void lcd_home(void) { lcd_set_cursor(0, 0); }
