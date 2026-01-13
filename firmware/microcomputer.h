#ifndef MICROCOMPUTER_H
#define MICROCOMPUTER_H

#include <stdint.h>
#include <stdbool.h>
#include "cpu8080.h"

typedef enum {
    MODE_STOP,
    MODE_RUN_SLOW,
    MODE_RUN_FAST
} run_mode_t;

#define INPUT_STOP_RUN_BIT1   0x001
#define INPUT_STOP_RUN_BIT2   0x002
#define INPUT_SINGLE_STEP     0x004
#define INPUT_RESET           0x008
#define INPUT_STORE_ADDR      0x010
#define INPUT_STORE_BYTE      0x020
#define INPUT_STORE_WORD      0x040
#define INPUT_AUTO_INC        0x080
#define INPUT_KEY_SWITCH      0x100

#define DEBOUNCE_MS 50

typedef struct {
    uint16_t current;
    uint16_t previous;
    uint32_t last_press_time[9];
} button_state_t;

typedef struct {
    cpu8080_t cpu;
    run_mode_t run_mode;
    bool auto_increment;
    button_state_t buttons;
    uint32_t last_step_time;
    uint32_t step_interval_ms;
    bool display_dirty;
    uint8_t cursor_pos;
    uint32_t last_cursor_time;
    bool show_registers;
} emulator_t;

void emulator_init(emulator_t *emu);
void emulator_update(emulator_t *emu, uint16_t switches, uint16_t buttons);

#endif
