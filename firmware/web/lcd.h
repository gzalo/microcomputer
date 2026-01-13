// Web-compatible LCD header
#ifndef LCD_H
#define LCD_H

#include <stdint.h>
#include <stdbool.h>

void lcd_init(void);
void lcd_clear(void);
void lcd_set_cursor(uint8_t col, uint8_t row);
void lcd_print(const char *str);
void lcd_putchar(char c);
void lcd_print_hex8(uint8_t value);
void lcd_print_hex16(uint16_t value);
void lcd_display(bool on, bool cursor, bool blink);
void lcd_command(uint8_t cmd);
void lcd_data(uint8_t data);
void lcd_home(void);

#endif
