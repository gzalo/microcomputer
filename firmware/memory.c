#include "memory.h"
#include <string.h>

static uint8_t ram[MEMORY_SIZE];

void memory_init(void) {
    memset(ram, 0, MEMORY_SIZE);
}

uint8_t memory_read(uint16_t addr) {
    return ram[addr];
}

void memory_write(uint16_t addr, uint8_t data) {
    ram[addr] = data;
}

uint16_t memory_read_word(uint16_t addr) {
    return ram[addr] | (ram[(uint16_t)(addr + 1)] << 8);
}

void memory_write_word(uint16_t addr, uint16_t data) {
    ram[addr] = data & 0xFF;
    ram[(uint16_t)(addr + 1)] = (data >> 8) & 0xFF;
}
