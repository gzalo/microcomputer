#ifndef DISASM_H
#define DISASM_H

#include <stdint.h>

int disasm_instruction(uint16_t addr, char *buffer, int buffer_size);
int disasm_get_length(uint16_t addr);

#endif
