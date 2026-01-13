#ifndef CPU8080_H
#define CPU8080_H

#include <stdint.h>
#include <stdbool.h>

#define FLAG_C  0x01
#define FLAG_P  0x04
#define FLAG_AC 0x10
#define FLAG_Z  0x40
#define FLAG_S  0x80

typedef struct {
    uint8_t a;
    uint8_t f;
    uint8_t b, c;
    uint8_t d, e;
    uint8_t h, l;
    uint16_t sp;
    uint16_t pc;
    bool halted;
    bool inte;
} cpu8080_t;

void cpu8080_init(cpu8080_t *cpu);
void cpu8080_reset(cpu8080_t *cpu);
int cpu8080_step(cpu8080_t *cpu);

static inline uint16_t cpu8080_get_bc(cpu8080_t *cpu) { return (cpu->b << 8) | cpu->c; }
static inline uint16_t cpu8080_get_de(cpu8080_t *cpu) { return (cpu->d << 8) | cpu->e; }
static inline uint16_t cpu8080_get_hl(cpu8080_t *cpu) { return (cpu->h << 8) | cpu->l; }

static inline void cpu8080_set_bc(cpu8080_t *cpu, uint16_t v) { cpu->b = v >> 8; cpu->c = v & 0xFF; }
static inline void cpu8080_set_de(cpu8080_t *cpu, uint16_t v) { cpu->d = v >> 8; cpu->e = v & 0xFF; }
static inline void cpu8080_set_hl(cpu8080_t *cpu, uint16_t v) { cpu->h = v >> 8; cpu->l = v & 0xFF; }

#endif
