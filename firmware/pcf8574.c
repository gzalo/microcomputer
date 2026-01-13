#include "pcf8574.h"
#include "pins.h"
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/gpio.h"

void pcf8574_init(void) {
    // Initialize I2C
    i2c_init(I2C_PORT, I2C_BAUDRATE);
    
    // Set up GPIO pins for I2C
    gpio_set_function(PIN_I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(PIN_I2C_SCL, GPIO_FUNC_I2C);
    
    // Enable pull-ups (PCF8574 needs them, but external ones are better)
    gpio_pull_up(PIN_I2C_SDA);
    gpio_pull_up(PIN_I2C_SCL);
    
    // Set PCF8574s to input mode by writing 0xFF (all high)
    // This enables the quasi-bidirectional I/O for reading
    pcf8574_write(PCF8574_ADDR_LOW, 0xFF);
    pcf8574_write(PCF8574_ADDR_HIGH, 0xFF);
}

uint8_t pcf8574_read(uint8_t addr) {
    uint8_t data = 0xFF;
    int result = i2c_read_blocking(I2C_PORT, addr, &data, 1, false);
    if (result < 0) {
        return 0xFF;  // Return all high on error (no switches pressed)
    }
    return data;
}

bool pcf8574_write(uint8_t addr, uint8_t data) {
    int result = i2c_write_blocking(I2C_PORT, addr, &data, 1, false);
    return (result == 1);
}

static uint8_t fix_bit_order(uint8_t data) {
    // Hardware wiring has bits scrambled:
    // Physical: bit3 bit7 bit6 bit5 bit4 bit2 bit1 bit0
    // Logical:  bit7 bit6 bit5 bit4 bit3 bit2 bit1 bit0
    // So: 3->7, 7->6, 6->5, 5->4, 4->3, bits 0-2 unchanged
    uint8_t result = data & 0x07;  // Keep bits 0-2
    result |= (data & 0x08) << 4;  // bit 3 -> bit 7
    result |= (data & 0x80) >> 1;  // bit 7 -> bit 6
    result |= (data & 0x40) >> 1;  // bit 6 -> bit 5
    result |= (data & 0x20) >> 1;  // bit 5 -> bit 4
    result |= (data & 0x10) >> 1;  // bit 4 -> bit 3
    return result;
}

uint16_t pcf8574_read_all(void) {
    uint8_t low = fix_bit_order(pcf8574_read(PCF8574_ADDR_LOW));
    uint8_t high = fix_bit_order(pcf8574_read(PCF8574_ADDR_HIGH));
    return ((uint16_t)high << 8) | low;
}

bool pcf8574_probe(uint8_t addr) {
    uint8_t dummy;
    int result = i2c_read_blocking(I2C_PORT, addr, &dummy, 1, false);
    return (result >= 0);
}
