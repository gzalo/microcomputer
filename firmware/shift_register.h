#ifndef SHIFT_REGISTER_H
#define SHIFT_REGISTER_H

#include <stdint.h>

// Initialize the 74HC595 shift register pins
void sr_init(void);

// Output 16 bits to the shift registers (controls 16 LEDs)
// LSB corresponds to first LED, MSB to last LED
void sr_output(uint16_t data);

#endif // SHIFT_REGISTER_H
