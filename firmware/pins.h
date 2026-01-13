#ifndef PINS_H
#define PINS_H

// =============================================================================
// Pin Definitions for Microcomputer Panel
// =============================================================================

// --- 74HC595 Shift Register (LED Output) ---
// Controls 16 LEDs (address/data display)
#define PIN_SR_DATA     2   // Serial data input (SER/DS)
#define PIN_SR_LATCH    3   // Latch clock (RCLK/ST_CP)
#define PIN_SR_CLOCK    4   // Shift clock (SRCLK/SH_CP)

// --- I2C for PCF8574 (16 address/data switches) ---
#define PIN_I2C_SDA     0   // I2C Data
#define PIN_I2C_SCL     1   // I2C Clock
#define I2C_PORT        i2c0
#define I2C_BAUDRATE    100000

// PCF8574 I2C addresses (depends on A0-A2 pins)
// Addresses: 0x20-0x27 for PCF8574, 0x38-0x3F for PCF8574A
#define PCF8574_ADDR_LOW    0x27    // Low 8 bits of address/data (PCF8574)
#define PCF8574_ADDR_HIGH   0x26    // High 8 bits of address/data (PCF8574)

// --- LCD Pins (4-bit mode) ---
#define PIN_LCD_D4      10  // LCD Data bit 4
#define PIN_LCD_D5      11  // LCD Data bit 5
#define PIN_LCD_D6      12  // LCD Data bit 6
#define PIN_LCD_D7      13  // LCD Data bit 7
#define PIN_LCD_RS      14  // Register Select (0=command, 1=data)
#define PIN_LCD_EN      15  // Enable pulse

// --- Direct Input Pins (8 switches/buttons) ---
#define PIN_SW_STOP_RUN     16  // 3-position: stop/run slow/run fast (1 of 2)
#define PIN_SW_RUN_SPEED    17  // 3-position: stop/run slow/run fast (2 of 2)
#define PIN_BTN_SINGLE_STEP 18  // Momentary: single step
#define PIN_BTN_RESET       19  // Momentary: reset
#define PIN_BTN_STORE_ADDR  20  // Momentary: store to address latch
#define PIN_BTN_STORE_BYTE  21  // Momentary: store byte
#define PIN_BTN_STORE_WORD  22  // Momentary: store word
#define PIN_SW_AUTO_INC     26  // Switch: auto increment Y/N
#define PIN_SW_KEY          27  // Switch: key

// Number of direct input pins
#define NUM_DIRECT_INPUTS   9

// Array of direct input pins for easy iteration
static const uint8_t direct_input_pins[] = {
    PIN_SW_STOP_RUN,
    PIN_SW_RUN_SPEED,
    PIN_BTN_SINGLE_STEP,
    PIN_BTN_RESET,
    PIN_BTN_STORE_ADDR,
    PIN_BTN_STORE_BYTE,
    PIN_BTN_STORE_WORD,
    PIN_SW_AUTO_INC,
    PIN_SW_KEY
};

// Names for direct inputs (for test display)
static const char* direct_input_names[] = {
    "STOP/RUN",
    "RUN SPEED",
    "STEP",
    "RESET",
    "ST ADDR",
    "ST BYTE",
    "ST WORD",
    "AUTO INC",
    "KEY"
};

#endif // PINS_H
