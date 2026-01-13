#ifndef MEMORY_H
#define MEMORY_H

#include <stdint.h>

#define MEMORY_SIZE 65536

void memory_init(void);
uint8_t memory_read(uint16_t addr);
void memory_write(uint16_t addr, uint8_t data);
uint16_t memory_read_word(uint16_t addr);
void memory_write_word(uint16_t addr, uint16_t data);

#endif
