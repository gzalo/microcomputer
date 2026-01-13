#ifndef LCD_H
#define LCD_H

#include <stdint.h>
#include <stdbool.h>

// LCD Commands
#define LCD_CMD_CLEAR           0x01
#define LCD_CMD_HOME            0x02
#define LCD_CMD_ENTRY_MODE      0x04
#define LCD_CMD_DISPLAY_CTRL    0x08
#define LCD_CMD_CURSOR_SHIFT    0x10
#define LCD_CMD_FUNCTION_SET    0x20
#define LCD_CMD_SET_CGRAM       0x40
#define LCD_CMD_SET_DDRAM       0x80

// Entry mode flags
#define LCD_ENTRY_INCREMENT     0x02
#define LCD_ENTRY_SHIFT         0x01

// Display control flags
#define LCD_DISPLAY_ON          0x04
#define LCD_CURSOR_ON           0x02
#define LCD_BLINK_ON            0x01

// Function set flags
#define LCD_8BIT_MODE           0x10
#define LCD_4BIT_MODE           0x00
#define LCD_2LINE               0x08
#define LCD_1LINE               0x00
#define LCD_5x10_DOTS           0x04
#define LCD_5x8_DOTS            0x00

// Initialize LCD in 4-bit mode
void lcd_init(void);

// Send a command to the LCD
void lcd_command(uint8_t cmd);

// Send data (character) to the LCD
void lcd_data(uint8_t data);

// Clear the display
void lcd_clear(void);

// Return cursor to home position
void lcd_home(void);

// Set cursor position (col: 0-15, row: 0-1)
void lcd_set_cursor(uint8_t col, uint8_t row);

// Print a string to the LCD
void lcd_print(const char* str);

// Print a single character
void lcd_putchar(char c);

// Print a hex byte (2 characters)
void lcd_print_hex8(uint8_t value);

// Print a hex word (4 characters)
void lcd_print_hex16(uint16_t value);

// Turn display on/off
void lcd_display(bool on, bool cursor, bool blink);

#endif // LCD_H
