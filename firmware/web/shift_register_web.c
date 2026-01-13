// Web shift register implementation
#include "shift_register.h"

uint16_t led_pattern = 0;

void sr_init(void) {
    led_pattern = 0;
}

void sr_output(uint16_t data) {
    led_pattern = data;
}
