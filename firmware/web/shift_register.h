// Web-compatible shift register header
#ifndef SHIFT_REGISTER_H
#define SHIFT_REGISTER_H

#include <stdint.h>

void sr_init(void);
void sr_output(uint16_t data);

#endif
