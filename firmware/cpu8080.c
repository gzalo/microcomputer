#include "cpu8080.h"
#include "memory.h"
#include <stddef.h>

void cpu8080_init(cpu8080_t *cpu) {
    cpu->a = cpu->f = 0;
    cpu->b = cpu->c = 0;
    cpu->d = cpu->e = 0;
    cpu->h = cpu->l = 0;
    cpu->sp = 0;
    cpu->pc = 0;
    cpu->halted = false;
    cpu->inte = false;
}

void cpu8080_reset(cpu8080_t *cpu) {
    cpu->pc = 0;
    cpu->halted = false;
}

static uint8_t parity(uint8_t v) {
    v ^= v >> 4;
    v ^= v >> 2;
    v ^= v >> 1;
    return (~v) & 1;
}

static void set_flags_zsp(cpu8080_t *cpu, uint8_t val) {
    cpu->f &= ~(FLAG_Z | FLAG_S | FLAG_P);
    if (val == 0) cpu->f |= FLAG_Z;
    if (val & 0x80) cpu->f |= FLAG_S;
    if (parity(val)) cpu->f |= FLAG_P;
}

static void set_flags_add(cpu8080_t *cpu, uint8_t a, uint8_t b, uint8_t cy) {
    uint16_t res = a + b + cy;
    set_flags_zsp(cpu, res & 0xFF);
    cpu->f &= ~(FLAG_C | FLAG_AC);
    if (res > 0xFF) cpu->f |= FLAG_C;
    if ((a & 0x0F) + (b & 0x0F) + cy > 0x0F) cpu->f |= FLAG_AC;
}

static void set_flags_sub(cpu8080_t *cpu, uint8_t a, uint8_t b, uint8_t cy) {
    uint16_t res = a - b - cy;
    set_flags_zsp(cpu, res & 0xFF);
    cpu->f &= ~(FLAG_C | FLAG_AC);
    if (res > 0xFF) cpu->f |= FLAG_C;
    if ((int)(a & 0x0F) - (int)(b & 0x0F) - (int)cy < 0) cpu->f |= FLAG_AC;
}

static uint8_t *get_reg(cpu8080_t *cpu, uint8_t r) {
    switch (r) {
        case 0: return &cpu->b;
        case 1: return &cpu->c;
        case 2: return &cpu->d;
        case 3: return &cpu->e;
        case 4: return &cpu->h;
        case 5: return &cpu->l;
        case 7: return &cpu->a;
        default: return NULL;
    }
}

static uint8_t read_reg(cpu8080_t *cpu, uint8_t r) {
    if (r == 6) return memory_read(cpu8080_get_hl(cpu));
    return *get_reg(cpu, r);
}

static void write_reg(cpu8080_t *cpu, uint8_t r, uint8_t val) {
    if (r == 6) memory_write(cpu8080_get_hl(cpu), val);
    else *get_reg(cpu, r) = val;
}

static uint8_t fetch(cpu8080_t *cpu) {
    return memory_read(cpu->pc++);
}

static uint16_t fetch16(cpu8080_t *cpu) {
    uint8_t lo = fetch(cpu);
    uint8_t hi = fetch(cpu);
    return (hi << 8) | lo;
}

static void push16(cpu8080_t *cpu, uint16_t val) {
    memory_write(--cpu->sp, val >> 8);
    memory_write(--cpu->sp, val & 0xFF);
}

static uint16_t pop16(cpu8080_t *cpu) {
    uint8_t lo = memory_read(cpu->sp++);
    uint8_t hi = memory_read(cpu->sp++);
    return (hi << 8) | lo;
}

static bool check_cond(cpu8080_t *cpu, uint8_t cond) {
    switch (cond) {
        case 0: return !(cpu->f & FLAG_Z);  // NZ
        case 1: return cpu->f & FLAG_Z;     // Z
        case 2: return !(cpu->f & FLAG_C);  // NC
        case 3: return cpu->f & FLAG_C;     // C
        case 4: return !(cpu->f & FLAG_P);  // PO
        case 5: return cpu->f & FLAG_P;     // PE
        case 6: return !(cpu->f & FLAG_S);  // P
        case 7: return cpu->f & FLAG_S;     // M
        default: return false;
    }
}

int cpu8080_step(cpu8080_t *cpu) {
    if (cpu->halted) return 0;

    uint8_t op = fetch(cpu);
    uint8_t hi = (op >> 6) & 0x03;
    uint8_t mid = (op >> 3) & 0x07;
    uint8_t lo = op & 0x07;

    // MOV r,r' / MOV r,M / MOV M,r / HLT
    if (hi == 1) {
        if (op == 0x76) {
            cpu->halted = true;
            return 7;
        }
        write_reg(cpu, mid, read_reg(cpu, lo));
        return (mid == 6 || lo == 6) ? 7 : 5;
    }

    // ALU operations with register
    if (hi == 2) {
        uint8_t val = read_reg(cpu, lo);
        uint8_t cy = (cpu->f & FLAG_C) ? 1 : 0;
        switch (mid) {
            case 0: set_flags_add(cpu, cpu->a, val, 0); cpu->a += val; break;
            case 1: { uint16_t r = cpu->a + val + cy; set_flags_add(cpu, cpu->a, val, cy); cpu->a = r; } break;
            case 2: set_flags_sub(cpu, cpu->a, val, 0); cpu->a -= val; break;
            case 3: { uint16_t r = cpu->a - val - cy; set_flags_sub(cpu, cpu->a, val, cy); cpu->a = r; } break;
            case 4: cpu->a &= val; set_flags_zsp(cpu, cpu->a); cpu->f &= ~FLAG_C; cpu->f |= FLAG_AC; break;
            case 5: cpu->a ^= val; set_flags_zsp(cpu, cpu->a); cpu->f &= ~(FLAG_C | FLAG_AC); break;
            case 6: cpu->a |= val; set_flags_zsp(cpu, cpu->a); cpu->f &= ~(FLAG_C | FLAG_AC); break;
            case 7: set_flags_sub(cpu, cpu->a, val, 0); break;  // CMP
        }
        return (lo == 6) ? 7 : 4;
    }

    switch (op) {
        case 0x00: return 4;  // NOP
        case 0x01: cpu8080_set_bc(cpu, fetch16(cpu)); return 10;
        case 0x11: cpu8080_set_de(cpu, fetch16(cpu)); return 10;
        case 0x21: cpu8080_set_hl(cpu, fetch16(cpu)); return 10;
        case 0x31: cpu->sp = fetch16(cpu); return 10;

        case 0x02: memory_write(cpu8080_get_bc(cpu), cpu->a); return 7;
        case 0x12: memory_write(cpu8080_get_de(cpu), cpu->a); return 7;
        case 0x0A: cpu->a = memory_read(cpu8080_get_bc(cpu)); return 7;
        case 0x1A: cpu->a = memory_read(cpu8080_get_de(cpu)); return 7;

        case 0x03: cpu8080_set_bc(cpu, cpu8080_get_bc(cpu) + 1); return 5;
        case 0x13: cpu8080_set_de(cpu, cpu8080_get_de(cpu) + 1); return 5;
        case 0x23: cpu8080_set_hl(cpu, cpu8080_get_hl(cpu) + 1); return 5;
        case 0x33: cpu->sp++; return 5;
        case 0x0B: cpu8080_set_bc(cpu, cpu8080_get_bc(cpu) - 1); return 5;
        case 0x1B: cpu8080_set_de(cpu, cpu8080_get_de(cpu) - 1); return 5;
        case 0x2B: cpu8080_set_hl(cpu, cpu8080_get_hl(cpu) - 1); return 5;
        case 0x3B: cpu->sp--; return 5;

        case 0x04: case 0x0C: case 0x14: case 0x1C:
        case 0x24: case 0x2C: case 0x34: case 0x3C: {
            uint8_t r = mid;
            uint8_t val = read_reg(cpu, r);
            uint8_t ac = ((val & 0x0F) == 0x0F) ? FLAG_AC : 0;
            val++;
            write_reg(cpu, r, val);
            set_flags_zsp(cpu, val);
            cpu->f = (cpu->f & FLAG_C) | (cpu->f & ~(FLAG_Z | FLAG_S | FLAG_P | FLAG_AC)) | ac;
            set_flags_zsp(cpu, val);
            cpu->f = (cpu->f & ~FLAG_AC) | ac;
            return (r == 6) ? 10 : 5;
        }
        case 0x05: case 0x0D: case 0x15: case 0x1D:
        case 0x25: case 0x2D: case 0x35: case 0x3D: {
            uint8_t r = mid;
            uint8_t val = read_reg(cpu, r);
            uint8_t ac = ((val & 0x0F) == 0x00) ? FLAG_AC : 0;
            val--;
            write_reg(cpu, r, val);
            set_flags_zsp(cpu, val);
            cpu->f = (cpu->f & ~FLAG_AC) | ac;
            return (r == 6) ? 10 : 5;
        }

        case 0x06: cpu->b = fetch(cpu); return 7;
        case 0x0E: cpu->c = fetch(cpu); return 7;
        case 0x16: cpu->d = fetch(cpu); return 7;
        case 0x1E: cpu->e = fetch(cpu); return 7;
        case 0x26: cpu->h = fetch(cpu); return 7;
        case 0x2E: cpu->l = fetch(cpu); return 7;
        case 0x36: memory_write(cpu8080_get_hl(cpu), fetch(cpu)); return 10;
        case 0x3E: cpu->a = fetch(cpu); return 7;

        case 0x07: {
            uint8_t cy = (cpu->a & 0x80) >> 7;
            cpu->a = (cpu->a << 1) | cy;
            cpu->f = (cpu->f & ~FLAG_C) | cy;
            return 4;
        }
        case 0x0F: {
            uint8_t cy = cpu->a & 0x01;
            cpu->a = (cpu->a >> 1) | (cy << 7);
            cpu->f = (cpu->f & ~FLAG_C) | cy;
            return 4;
        }
        case 0x17: {
            uint8_t cy = (cpu->a & 0x80) >> 7;
            cpu->a = (cpu->a << 1) | ((cpu->f & FLAG_C) ? 1 : 0);
            cpu->f = (cpu->f & ~FLAG_C) | cy;
            return 4;
        }
        case 0x1F: {
            uint8_t cy = cpu->a & 0x01;
            cpu->a = (cpu->a >> 1) | ((cpu->f & FLAG_C) ? 0x80 : 0);
            cpu->f = (cpu->f & ~FLAG_C) | cy;
            return 4;
        }

        case 0x09: {
            uint32_t r = cpu8080_get_hl(cpu) + cpu8080_get_bc(cpu);
            cpu8080_set_hl(cpu, r);
            cpu->f = (cpu->f & ~FLAG_C) | ((r > 0xFFFF) ? FLAG_C : 0);
            return 10;
        }
        case 0x19: {
            uint32_t r = cpu8080_get_hl(cpu) + cpu8080_get_de(cpu);
            cpu8080_set_hl(cpu, r);
            cpu->f = (cpu->f & ~FLAG_C) | ((r > 0xFFFF) ? FLAG_C : 0);
            return 10;
        }
        case 0x29: {
            uint32_t r = cpu8080_get_hl(cpu) + cpu8080_get_hl(cpu);
            cpu8080_set_hl(cpu, r);
            cpu->f = (cpu->f & ~FLAG_C) | ((r > 0xFFFF) ? FLAG_C : 0);
            return 10;
        }
        case 0x39: {
            uint32_t r = cpu8080_get_hl(cpu) + cpu->sp;
            cpu8080_set_hl(cpu, r);
            cpu->f = (cpu->f & ~FLAG_C) | ((r > 0xFFFF) ? FLAG_C : 0);
            return 10;
        }

        case 0x22: memory_write_word(fetch16(cpu), cpu8080_get_hl(cpu)); return 16;
        case 0x2A: cpu8080_set_hl(cpu, memory_read_word(fetch16(cpu))); return 16;
        case 0x32: memory_write(fetch16(cpu), cpu->a); return 13;
        case 0x3A: cpu->a = memory_read(fetch16(cpu)); return 13;

        case 0x2F: cpu->a = ~cpu->a; return 4;
        case 0x37: cpu->f |= FLAG_C; return 4;
        case 0x3F: cpu->f ^= FLAG_C; return 4;

        case 0x27: {
            uint8_t cy = cpu->f & FLAG_C;
            uint8_t add = 0;
            if ((cpu->f & FLAG_AC) || (cpu->a & 0x0F) > 9) add |= 0x06;
            if (cy || cpu->a > 0x99) { add |= 0x60; cy = FLAG_C; }
            set_flags_add(cpu, cpu->a, add, 0);
            cpu->a += add;
            cpu->f |= cy;
            return 4;
        }

        case 0xC6: { uint8_t v = fetch(cpu); set_flags_add(cpu, cpu->a, v, 0); cpu->a += v; return 7; }
        case 0xCE: { uint8_t v = fetch(cpu); uint8_t c = (cpu->f & FLAG_C) ? 1 : 0; set_flags_add(cpu, cpu->a, v, c); cpu->a += v + c; return 7; }
        case 0xD6: { uint8_t v = fetch(cpu); set_flags_sub(cpu, cpu->a, v, 0); cpu->a -= v; return 7; }
        case 0xDE: { uint8_t v = fetch(cpu); uint8_t c = (cpu->f & FLAG_C) ? 1 : 0; set_flags_sub(cpu, cpu->a, v, c); cpu->a -= v + c; return 7; }
        case 0xE6: { uint8_t v = fetch(cpu); cpu->a &= v; set_flags_zsp(cpu, cpu->a); cpu->f &= ~FLAG_C; cpu->f |= FLAG_AC; return 7; }
        case 0xEE: { uint8_t v = fetch(cpu); cpu->a ^= v; set_flags_zsp(cpu, cpu->a); cpu->f &= ~(FLAG_C | FLAG_AC); return 7; }
        case 0xF6: { uint8_t v = fetch(cpu); cpu->a |= v; set_flags_zsp(cpu, cpu->a); cpu->f &= ~(FLAG_C | FLAG_AC); return 7; }
        case 0xFE: { uint8_t v = fetch(cpu); set_flags_sub(cpu, cpu->a, v, 0); return 7; }

        case 0xC3: cpu->pc = fetch16(cpu); return 10;
        case 0xC2: case 0xCA: case 0xD2: case 0xDA:
        case 0xE2: case 0xEA: case 0xF2: case 0xFA: {
            uint16_t addr = fetch16(cpu);
            if (check_cond(cpu, mid)) cpu->pc = addr;
            return 10;
        }

        case 0xCD: { uint16_t addr = fetch16(cpu); push16(cpu, cpu->pc); cpu->pc = addr; return 17; }
        case 0xC4: case 0xCC: case 0xD4: case 0xDC:
        case 0xE4: case 0xEC: case 0xF4: case 0xFC: {
            uint16_t addr = fetch16(cpu);
            if (check_cond(cpu, mid)) { push16(cpu, cpu->pc); cpu->pc = addr; return 17; }
            return 11;
        }

        case 0xC9: cpu->pc = pop16(cpu); return 10;
        case 0xC0: case 0xC8: case 0xD0: case 0xD8:
        case 0xE0: case 0xE8: case 0xF0: case 0xF8:
            if (check_cond(cpu, mid)) { cpu->pc = pop16(cpu); return 11; }
            return 5;

        case 0xC7: case 0xCF: case 0xD7: case 0xDF:
        case 0xE7: case 0xEF: case 0xF7: case 0xFF:
            push16(cpu, cpu->pc);
            cpu->pc = mid << 3;
            return 11;

        case 0xC5: push16(cpu, cpu8080_get_bc(cpu)); return 11;
        case 0xD5: push16(cpu, cpu8080_get_de(cpu)); return 11;
        case 0xE5: push16(cpu, cpu8080_get_hl(cpu)); return 11;
        case 0xF5: push16(cpu, (cpu->a << 8) | (cpu->f | 0x02)); return 11;

        case 0xC1: cpu8080_set_bc(cpu, pop16(cpu)); return 10;
        case 0xD1: cpu8080_set_de(cpu, pop16(cpu)); return 10;
        case 0xE1: cpu8080_set_hl(cpu, pop16(cpu)); return 10;
        case 0xF1: { uint16_t v = pop16(cpu); cpu->f = (v & 0xD7) | 0x02; cpu->a = v >> 8; return 10; }

        case 0xE3: {
            uint16_t tmp = memory_read_word(cpu->sp);
            memory_write_word(cpu->sp, cpu8080_get_hl(cpu));
            cpu8080_set_hl(cpu, tmp);
            return 18;
        }
        case 0xE9: cpu->pc = cpu8080_get_hl(cpu); return 5;
        case 0xEB: {
            uint16_t tmp = cpu8080_get_de(cpu);
            cpu8080_set_de(cpu, cpu8080_get_hl(cpu));
            cpu8080_set_hl(cpu, tmp);
            return 4;
        }
        case 0xF9: cpu->sp = cpu8080_get_hl(cpu); return 5;

        case 0xDB: fetch(cpu); return 10;  // IN (stub)
        case 0xD3: fetch(cpu); return 10;  // OUT (stub)

        case 0xFB: cpu->inte = true; return 4;
        case 0xF3: cpu->inte = false; return 4;

        default: return 4;  // Undefined opcodes as NOP
    }
}
