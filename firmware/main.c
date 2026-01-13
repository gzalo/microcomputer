/**
 * Microcomputer - 8080 Emulator
 *
 * An Altair/IMSAI style microcomputer with:
 * - 8080 CPU emulator
 * - 64KB RAM
 * - 16 LEDs showing address high byte and data bus
 * - 16x2 LCD showing disassembly or registers
 * - Front panel controls for memory examine/modify
 *
 * Test mode runs only on startup if key switch is OFF.
 * Key switch toggles LCD between disassembly and register view.
 */

#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "tusb.h"

#include "pins.h"
#include "lcd.h"
#include "shift_register.h"
#include "pcf8574.h"
#include "microcomputer.h"

// Initialize all direct input pins
void init_direct_inputs(void) {
    for (int i = 0; i < NUM_DIRECT_INPUTS; i++) {
        gpio_init(direct_input_pins[i]);
        gpio_set_dir(direct_input_pins[i], GPIO_IN);
        gpio_pull_up(direct_input_pins[i]);
    }
}

// Read all direct inputs as a bitmask
uint16_t read_direct_inputs(void) {
    uint16_t result = 0;
    for (int i = 0; i < NUM_DIRECT_INPUTS; i++) {
        if (!gpio_get(direct_input_pins[i])) {
            result |= (1 << i);
        }
    }
    return result;
}

// Test mode - LED chase with input display (runs once on startup)
void run_test(void) {
    lcd_clear();
    lcd_set_cursor(0, 0);
    lcd_print("Test Mode");

    uint16_t last_switches = 0xFFFF;
    uint16_t last_buttons = 0xFFFF;

    for (int round = 0; round < 2; round++) {
        for (int i = 0; i < 16; i++) {
            uint16_t pattern = 1 << i;
            sr_output(pattern);

            uint16_t switches = ~pcf8574_read_all();
            uint16_t buttons = read_direct_inputs();

            if (switches != last_switches || buttons != last_buttons) {
                lcd_set_cursor(0, 1);
                lcd_print("SW:");
                lcd_print_hex16(switches);
                lcd_print(" B:");
                lcd_print_hex16(buttons);
                last_switches = switches;
                last_buttons = buttons;
            }

            sleep_ms(100);
        }
    }

    sr_output(0x0000);
}

static emulator_t emu;

int main() {
    stdio_init_all();

    #define PICO_DEFAULT_LED_PIN 25
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);

    sr_init();
    pcf8574_init();
    init_direct_inputs();
    lcd_init();

    lcd_clear();
    lcd_set_cursor(0, 0);
    lcd_print("8080 Emulator");
    lcd_set_cursor(0, 1);
    lcd_print("Ready");
    sleep_ms(1000);

    // Run test mode only on startup if key is OFF
    uint16_t buttons = read_direct_inputs();
    if (!(buttons & INPUT_KEY_SWITCH)) {
        run_test();
    }

    // Initialize emulator
    emulator_init(&emu);

    while (1) {
        buttons = read_direct_inputs();
        uint16_t switches = ~pcf8574_read_all();

        emulator_update(&emu, switches, buttons);

        sleep_ms(10);
    }

    return 0;
}
