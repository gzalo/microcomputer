// Stub header for pico/stdlib.h - web build
#ifndef PICO_STDLIB_H
#define PICO_STDLIB_H

#include <stdint.h>

// These are defined in main_web.c
uint32_t to_ms_since_boot(uint64_t t);
uint64_t get_absolute_time(void);
void sleep_ms(uint32_t ms);

#endif
