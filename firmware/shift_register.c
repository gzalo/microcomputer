#include "shift_register.h"
#include "pins.h"
#include "pico/stdlib.h"
#include "hardware/gpio.h"

void sr_init(void) {
    // Initialize GPIO pins
    gpio_init(PIN_SR_DATA);
    gpio_init(PIN_SR_CLOCK);
    gpio_init(PIN_SR_LATCH);
    
    gpio_set_dir(PIN_SR_DATA, GPIO_OUT);
    gpio_set_dir(PIN_SR_CLOCK, GPIO_OUT);
    gpio_set_dir(PIN_SR_LATCH, GPIO_OUT);
    
    // Start with all outputs low
    gpio_put(PIN_SR_DATA, 0);
    gpio_put(PIN_SR_CLOCK, 0);
    gpio_put(PIN_SR_LATCH, 0);
    
    // Clear the shift registers
    sr_output(0x0000);
}

static uint8_t reverse_bits(uint8_t data) {
    // Reverse bit order: bit 7 <-> bit 0, bit 6 <-> bit 1, etc.
    data = ((data & 0xF0) >> 4) | ((data & 0x0F) << 4);
    data = ((data & 0xCC) >> 2) | ((data & 0x33) << 2);
    data = ((data & 0xAA) >> 1) | ((data & 0x55) << 1);
    return data;
}

static uint8_t rotate_left(uint8_t data) {
    // Rotate left by 1: bit 7 moves to bit 0, all others shift up
    return (data << 1) | (data >> 7);
}

void sr_output8(uint8_t data) {
    // Reverse bit order so leftmost LED is MSB
    data = reverse_bits(data);
    // Rotate left by 1 for hardware wiring offset
    data = rotate_left(data);

    // Shift out 8 bits, MSB first
    for (int i = 7; i >= 0; i--) {
        // Set data bit
        gpio_put(PIN_SR_DATA, (data >> i) & 0x01);
        sleep_us(1);
        
        // Pulse clock
        gpio_put(PIN_SR_CLOCK, 1);
        sleep_us(1);
        gpio_put(PIN_SR_CLOCK, 0);
        sleep_us(1);
    }
}

void sr_output(uint16_t data) {
    // Shift out 16 bits (high byte first so it ends up in second register)
    sr_output8((data >> 8) & 0xFF);  // High byte
    sr_output8(data & 0xFF);          // Low byte
    
    // Pulse latch to transfer shift register to output
    gpio_put(PIN_SR_LATCH, 1);
    sleep_us(1);
    gpio_put(PIN_SR_LATCH, 0);
}
