#ifndef PROGRAMS_H
#define PROGRAMS_H

#include <stdint.h>

// Example program: Counter
// Continuously increments A register, visible on data LEDs
// Load at address 0x0000
//
// 0000: MVI A, 00    ; A = 0
// 0002: INR A        ; A = A + 1
// 0003: JMP 0002     ; Loop forever
//
static const uint8_t prog_counter[] = {
    0x3E, 0x00,       // MVI A, 0x00
    0x3C,             // INR A
    0xC3, 0x02, 0x00  // JMP 0x0002
};
#define PROG_COUNTER_ADDR 0x0000
#define PROG_COUNTER_SIZE sizeof(prog_counter)

// Example program: Memory fill
// Fills memory from 0x1000 with incrementing values
// Load at address 0x0000
//
// 0000: LXI H, 1000  ; HL = 0x1000
// 0003: MVI A, 00    ; A = 0
// 0005: MOV M, A     ; [HL] = A
// 0006: INR A        ; A++
// 0007: INX H        ; HL++
// 0008: JMP 0005     ; Loop
//
static const uint8_t prog_memfill[] = {
    0x21, 0x00, 0x10, // LXI H, 0x1000
    0x3E, 0x00,       // MVI A, 0x00
    0x77,             // MOV M, A
    0x3C,             // INR A
    0x23,             // INX H
    0xC3, 0x05, 0x00  // JMP 0x0005
};
#define PROG_MEMFILL_ADDR 0x0000
#define PROG_MEMFILL_SIZE sizeof(prog_memfill)

// Example program: Fibonacci
// Computes Fibonacci sequence in B, C registers
// Load at address 0x0000
//
// 0000: MVI B, 00    ; B = 0 (F_n-1)
// 0002: MVI C, 01    ; C = 1 (F_n)
// 0004: MOV A, B     ; A = B
// 0005: ADD C        ; A = A + C
// 0006: MOV B, C     ; B = C (shift)
// 0007: MOV C, A     ; C = A (new Fib)
// 0008: JMP 0004     ; Loop (overflows after F_13=233)
//
static const uint8_t prog_fibonacci[] = {
    0x06, 0x00,       // MVI B, 0x00
    0x0E, 0x01,       // MVI C, 0x01
    0x78,             // MOV A, B
    0x81,             // ADD C
    0x41,             // MOV B, C
    0x4F,             // MOV C, A
    0xC3, 0x04, 0x00  // JMP 0x0004
};
#define PROG_FIBONACCI_ADDR 0x0000
#define PROG_FIBONACCI_SIZE sizeof(prog_fibonacci)

// Example program: Delay loop with visible counting
// Counts 0-255 with delay, shows on A register
// Load at address 0x0000
//
// 0000: MVI A, 00    ; A = 0
// 0002: MVI B, FF    ; B = 255 (outer delay)
// 0004: MVI C, FF    ; C = 255 (inner delay)
// 0006: DCR C        ; C--
// 0007: JNZ 0006     ; inner loop
// 000A: DCR B        ; B--
// 000B: JNZ 0004     ; outer loop
// 000E: INR A        ; A++ (visible count)
// 000F: JMP 0002     ; repeat
//
static const uint8_t prog_delay_count[] = {
    0x3E, 0x00,       // MVI A, 0x00
    0x06, 0xFF,       // MVI B, 0xFF
    0x0E, 0xFF,       // MVI C, 0xFF
    0x0D,             // DCR C
    0xC2, 0x06, 0x00, // JNZ 0x0006
    0x05,             // DCR B
    0xC2, 0x04, 0x00, // JNZ 0x0004
    0x3C,             // INR A
    0xC3, 0x02, 0x00  // JMP 0x0002
};
#define PROG_DELAY_COUNT_ADDR 0x0000
#define PROG_DELAY_COUNT_SIZE sizeof(prog_delay_count)

// Example program: Stack test
// Tests PUSH/POP and CALL/RET
// Load at address 0x0000
//
// 0000: LXI SP, 2000 ; SP = 0x2000
// 0003: MVI A, 55    ; A = 0x55
// 0005: MVI B, AA    ; B = 0xAA
// 0007: PUSH PSW     ; Push A and flags
// 0008: PUSH B       ; Push BC
// 0009: CALL 0010    ; Call subroutine
// 000C: JMP 0003     ; Loop
// 000F: NOP          ; padding
// 0010: POP B        ; Pop into BC
// 0011: POP PSW      ; Pop into A/flags
// 0012: RET          ; Return
//
static const uint8_t prog_stack_test[] = {
    0x31, 0x00, 0x20, // LXI SP, 0x2000
    0x3E, 0x55,       // MVI A, 0x55
    0x06, 0xAA,       // MVI B, 0xAA
    0xF5,             // PUSH PSW
    0xC5,             // PUSH B
    0xCD, 0x10, 0x00, // CALL 0x0010
    0xC3, 0x03, 0x00, // JMP 0x0003
    0x00,             // NOP (padding)
    0xC1,             // POP B
    0xF1,             // POP PSW
    0xC9              // RET
};
#define PROG_STACK_TEST_ADDR 0x0000
#define PROG_STACK_TEST_SIZE sizeof(prog_stack_test)

// Helper to load a program into memory
#include "memory.h"
static inline void load_program(uint16_t addr, const uint8_t *prog, uint16_t size) {
    for (uint16_t i = 0; i < size; i++) {
        memory_write(addr + i, prog[i]);
    }
}

#endif // PROGRAMS_H
