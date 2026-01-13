#include "lcd.h"
#include "pins.h"
#include "pico/stdlib.h"
#include "hardware/gpio.h"

// Internal functions
static void lcd_pulse_enable(void);
static void lcd_send_nibble(uint8_t nibble);
static void lcd_send_byte(uint8_t byte, bool rs);

// Pulse the enable pin to latch data
static void lcd_pulse_enable(void) {
    gpio_put(PIN_LCD_EN, 1);
    sleep_us(1);    // Enable pulse width > 450ns
    gpio_put(PIN_LCD_EN, 0);
    sleep_us(50);   // Commands need > 37us to execute
}

// Send 4 bits to LCD
static void lcd_send_nibble(uint8_t nibble) {
    gpio_put(PIN_LCD_D4, (nibble >> 0) & 0x01);
    gpio_put(PIN_LCD_D5, (nibble >> 1) & 0x01);
    gpio_put(PIN_LCD_D6, (nibble >> 2) & 0x01);
    gpio_put(PIN_LCD_D7, (nibble >> 3) & 0x01);
    lcd_pulse_enable();
}

// Send a byte to LCD (as two nibbles in 4-bit mode)
static void lcd_send_byte(uint8_t byte, bool rs) {
    gpio_put(PIN_LCD_RS, rs);
    lcd_send_nibble(byte >> 4);     // High nibble first
    lcd_send_nibble(byte & 0x0F);   // Low nibble second
}

// Initialize the LCD
void lcd_init(void) {
    // Initialize GPIO pins
    gpio_init(PIN_LCD_D4);
    gpio_init(PIN_LCD_D5);
    gpio_init(PIN_LCD_D6);
    gpio_init(PIN_LCD_D7);
    gpio_init(PIN_LCD_RS);
    gpio_init(PIN_LCD_EN);
    
    gpio_set_dir(PIN_LCD_D4, GPIO_OUT);
    gpio_set_dir(PIN_LCD_D5, GPIO_OUT);
    gpio_set_dir(PIN_LCD_D6, GPIO_OUT);
    gpio_set_dir(PIN_LCD_D7, GPIO_OUT);
    gpio_set_dir(PIN_LCD_RS, GPIO_OUT);
    gpio_set_dir(PIN_LCD_EN, GPIO_OUT);
    
    // All outputs low initially
    gpio_put(PIN_LCD_D4, 0);
    gpio_put(PIN_LCD_D5, 0);
    gpio_put(PIN_LCD_D6, 0);
    gpio_put(PIN_LCD_D7, 0);
    gpio_put(PIN_LCD_RS, 0);
    gpio_put(PIN_LCD_EN, 0);
    
    // Wait for LCD to power up
    sleep_ms(50);
    
    // Initialization sequence for 4-bit mode (per HD44780 datasheet)
    gpio_put(PIN_LCD_RS, 0);
    
    // First: send 0x03 three times
    lcd_send_nibble(0x03);
    sleep_ms(5);
    lcd_send_nibble(0x03);
    sleep_us(150);
    lcd_send_nibble(0x03);
    sleep_us(150);
    
    // Switch to 4-bit mode
    lcd_send_nibble(0x02);
    sleep_us(150);
    
    // Now in 4-bit mode, configure LCD
    lcd_command(LCD_CMD_FUNCTION_SET | LCD_4BIT_MODE | LCD_2LINE | LCD_5x8_DOTS);
    lcd_command(LCD_CMD_DISPLAY_CTRL | LCD_DISPLAY_ON);
    lcd_command(LCD_CMD_CLEAR);
    sleep_ms(2);
    lcd_command(LCD_CMD_ENTRY_MODE | LCD_ENTRY_INCREMENT);
}

void lcd_command(uint8_t cmd) {
    lcd_send_byte(cmd, false);
    if (cmd == LCD_CMD_CLEAR || cmd == LCD_CMD_HOME) {
        sleep_ms(2);  // These commands take longer
    }
}

void lcd_data(uint8_t data) {
    lcd_send_byte(data, true);
}

void lcd_clear(void) {
    lcd_command(LCD_CMD_CLEAR);
}

void lcd_home(void) {
    lcd_command(LCD_CMD_HOME);
}

void lcd_set_cursor(uint8_t col, uint8_t row) {
    uint8_t addr = col + (row == 0 ? 0x00 : 0x40);
    lcd_command(LCD_CMD_SET_DDRAM | addr);
}

void lcd_print(const char* str) {
    while (*str) {
        lcd_data(*str++);
    }
}

void lcd_putchar(char c) {
    lcd_data(c);
}

void lcd_print_hex8(uint8_t value) {
    static const char hex[] = "0123456789ABCDEF";
    lcd_data(hex[(value >> 4) & 0x0F]);
    lcd_data(hex[value & 0x0F]);
}

void lcd_print_hex16(uint16_t value) {
    lcd_print_hex8((value >> 8) & 0xFF);
    lcd_print_hex8(value & 0xFF);
}

void lcd_display(bool on, bool cursor, bool blink) {
    uint8_t cmd = LCD_CMD_DISPLAY_CTRL;
    if (on) cmd |= LCD_DISPLAY_ON;
    if (cursor) cmd |= LCD_CURSOR_ON;
    if (blink) cmd |= LCD_BLINK_ON;
    lcd_command(cmd);
}
