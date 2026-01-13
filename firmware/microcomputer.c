#include "microcomputer.h"
#include "memory.h"
#include "disasm.h"
#include "lcd.h"
#include "shift_register.h"
#include "programs.h"
#include "pico/stdlib.h"

static void button_update(button_state_t *state, uint16_t current) {
    state->previous = state->current;
    state->current = current;
}

static bool button_pressed(button_state_t *state, uint16_t mask, int bit_index, uint32_t now) {
    bool is_pressed = (state->current & mask) != 0;
    bool was_pressed = (state->previous & mask) != 0;

    if (is_pressed && !was_pressed) {
        if (now - state->last_press_time[bit_index] >= DEBOUNCE_MS) {
            state->last_press_time[bit_index] = now;
            return true;
        }
    }
    return false;
}

void emulator_init(emulator_t *emu) {
    cpu8080_init(&emu->cpu);
    memory_init();
    emu->run_mode = MODE_STOP;
    emu->auto_increment = false;
    emu->buttons.current = 0;
    emu->buttons.previous = 0;
    for (int i = 0; i < 9; i++) {
        emu->buttons.last_press_time[i] = 0;
    }
    emu->last_step_time = 0;
    emu->step_interval_ms = 300;
    emu->display_dirty = true;
    emu->cursor_pos = 0;
    emu->last_cursor_time = 0;
    emu->show_registers = false;
}

static void update_leds(emulator_t *emu) {
    uint8_t data = memory_read(emu->cpu.pc);
    uint16_t pattern = ((emu->cpu.pc >> 8) << 8) | data;
    sr_output(pattern);
}

static void update_lcd_registers(emulator_t *emu) {
    // Show CPU registers (20x2 display)
    // Line 1: A:xx BC:xxxx DE:xxxx (19 chars)
    // Line 2: HL:xxxx SP:xxxx F:xx (19 chars)
    lcd_set_cursor(0, 0);
    lcd_print("A:");
    lcd_print_hex8(emu->cpu.a);
    lcd_print(" BC:");
    lcd_print_hex16((emu->cpu.b << 8) | emu->cpu.c);
    lcd_print(" DE:");
    lcd_print_hex16((emu->cpu.d << 8) | emu->cpu.e);

    lcd_set_cursor(0, 1);
    lcd_print("HL:");
    lcd_print_hex16((emu->cpu.h << 8) | emu->cpu.l);
    lcd_print(" SP:");
    lcd_print_hex16(emu->cpu.sp);
    lcd_print(" F:");
    lcd_print_hex8(emu->cpu.f);

    lcd_display(true, false, false);  // Display on, cursor off
}

static void update_lcd_disasm(emulator_t *emu, uint32_t now) {
    uint16_t addr = emu->cpu.pc;
    int instr_len = disasm_get_length(addr);

    if (emu->display_dirty) {
        emu->display_dirty = false;
        emu->cursor_pos = 0;
        emu->last_cursor_time = now;

        char disasm_buf[12];

        lcd_clear();
        lcd_set_cursor(0, 0);
        lcd_print_hex16(addr);
        lcd_print(": ");
        disasm_instruction(addr, disasm_buf, sizeof(disasm_buf));
        lcd_print(disasm_buf);

        lcd_set_cursor(0, 1);
        for (int i = 0; i < 7; i++) {
            if (i > 0) lcd_putchar('.');
            lcd_print_hex8(memory_read(addr + i));
        }
    }

    // Animate cursor every 100ms
    if (now - emu->last_cursor_time >= 100) {
        emu->last_cursor_time = now;
        emu->cursor_pos++;
        // For 1-byte instruction: cycle through positions 0,1 (2 chars)
        // For 2-byte instruction: cycle through positions 0,1,3,4 (4 chars)
        // For 3-byte instruction: cycle through positions 0,1,3,4,6,7 (6 chars)
        if (emu->cursor_pos >= instr_len * 2) {
            emu->cursor_pos = 0;
        }
    }

    // Calculate cursor column: each byte is 2 chars + 1 dot separator
    // Byte 0: cols 0,1 | Byte 1: cols 3,4 | Byte 2: cols 6,7
    uint8_t byte_idx = emu->cursor_pos / 2;
    uint8_t char_in_byte = emu->cursor_pos % 2;
    uint8_t col = byte_idx * 3 + char_in_byte;

    lcd_set_cursor(col, 1);
    lcd_display(true, true, false);  // Display on, cursor on
}

static void update_lcd(emulator_t *emu, uint32_t now) {
    // Check if display mode changed
    bool key_off = !(emu->buttons.current & INPUT_KEY_SWITCH);
    if (key_off != emu->show_registers) {
        emu->show_registers = key_off;
        lcd_clear();
        emu->display_dirty = true;
    }

    if (emu->show_registers) {
        update_lcd_registers(emu);
    } else {
        update_lcd_disasm(emu, now);
    }
}

void emulator_update(emulator_t *emu, uint16_t switches, uint16_t buttons) {
    uint32_t now = to_ms_since_boot(get_absolute_time());

    button_update(&emu->buttons, buttons);

    emu->auto_increment = (buttons & INPUT_AUTO_INC) == 0;

    uint8_t run_bits = buttons & 0x03;
    if (run_bits == 0x00) {
        emu->run_mode = MODE_STOP;
    } else if (run_bits == 0x01) {
        emu->run_mode = MODE_RUN_SLOW;
        emu->step_interval_ms = 300;
    } else {
        emu->run_mode = MODE_RUN_FAST;
        emu->step_interval_ms = 10;
    }

    if (button_pressed(&emu->buttons, INPUT_RESET, 3, now)) {
        cpu8080_reset(&emu->cpu);
        // Load test program based on switch value (low byte)
        // 0x01 = Counter, 0x02 = Memfill, 0x03 = Fibonacci
        // 0x04 = Delay count, 0x05 = Stack test
        const char *prog_name = NULL;
        uint8_t prog_select = switches & 0xFF;
        switch (prog_select) {
            case 0x01:
                load_program(PROG_COUNTER_ADDR, prog_counter, PROG_COUNTER_SIZE);
                prog_name = "Counter";
                break;
            case 0x02:
                load_program(PROG_MEMFILL_ADDR, prog_memfill, PROG_MEMFILL_SIZE);
                prog_name = "Memfill";
                break;
            case 0x03:
                load_program(PROG_FIBONACCI_ADDR, prog_fibonacci, PROG_FIBONACCI_SIZE);
                prog_name = "Fibonacci";
                break;
            case 0x04:
                load_program(PROG_DELAY_COUNT_ADDR, prog_delay_count, PROG_DELAY_COUNT_SIZE);
                prog_name = "Delay Count";
                break;
            case 0x05:
                load_program(PROG_STACK_TEST_ADDR, prog_stack_test, PROG_STACK_TEST_SIZE);
                prog_name = "Stack Test";
                break;
            default:
                // No program loaded, just reset
                break;
        }
        if (prog_name) {
            lcd_clear();
            lcd_set_cursor(0, 0);
            lcd_print("Loaded: ");
            lcd_print(prog_name);
            sleep_ms(500);
        }
        emu->display_dirty = true;
    }

    if (emu->run_mode == MODE_STOP) {
        if (button_pressed(&emu->buttons, INPUT_SINGLE_STEP, 2, now)) {
            if (!emu->cpu.halted) {
                cpu8080_step(&emu->cpu);
                emu->display_dirty = true;
            }
        }
    }

    if (button_pressed(&emu->buttons, INPUT_STORE_ADDR, 4, now)) {
        emu->cpu.pc = switches;
        emu->display_dirty = true;
    }

    if (button_pressed(&emu->buttons, INPUT_STORE_BYTE, 5, now)) {
        memory_write(emu->cpu.pc, switches & 0xFF);
        if (emu->auto_increment) {
            emu->cpu.pc++;
        }
        emu->display_dirty = true;
    }

    if (button_pressed(&emu->buttons, INPUT_STORE_WORD, 6, now)) {
        memory_write_word(emu->cpu.pc, switches);
        if (emu->auto_increment) {
            emu->cpu.pc += 2;
        }
        emu->display_dirty = true;
    }

    if (emu->run_mode != MODE_STOP && !emu->cpu.halted) {
        if (now - emu->last_step_time >= emu->step_interval_ms) {
            cpu8080_step(&emu->cpu);
            emu->display_dirty = true;
            emu->last_step_time = now;
        }
    }

    update_leds(emu);
    update_lcd(emu, now);
}
