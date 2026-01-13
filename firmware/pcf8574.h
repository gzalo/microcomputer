#ifndef PCF8574_H
#define PCF8574_H

#include <stdint.h>
#include <stdbool.h>

// Initialize I2C for PCF8574 communication
void pcf8574_init(void);

// Read 8 bits from a PCF8574 at the given address
// Returns the input state (0xFF if read fails)
uint8_t pcf8574_read(uint8_t addr);

// Write 8 bits to a PCF8574 at the given address
// Returns true on success
bool pcf8574_write(uint8_t addr, uint8_t data);

// Read all 16 bits from both PCF8574s (address/data switches)
// Returns combined value: high byte from HIGH addr, low byte from LOW addr
uint16_t pcf8574_read_all(void);

// Check if a PCF8574 is present at the given address
bool pcf8574_probe(uint8_t addr);

#endif // PCF8574_H
