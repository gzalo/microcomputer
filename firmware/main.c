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
#include "memory.h"

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
static uint32_t challenge_notice_until;
static uint32_t challenge_notice_started;
static bool challenge_halt_seen;
static uint8_t challenge_notice_id;

#define CHALLENGE_NOTICE_MS 12000
#define CHALLENGE_NOTICE_PAGE_MS 3000

static uint8_t completed_challenge(void) {
    if (emu.cpu.a == 0x42) return 1;
    if (memory_read(0x4200) == 0x3C) return 2;
    if (memory_read(0x4300) == 0x10 && memory_read(0x4301) == 0x11 &&
        memory_read(0x4302) == 0x12 && memory_read(0x4303) == 0x13) return 3;
    return 0;
}

static void lcd_print_line(uint8_t row, const char *text) {
    lcd_set_cursor(0, row);
    lcd_print(text);
    for (size_t i = strlen(text); i < 16; i++) {
        lcd_putchar(' ');
    }
}

static void show_challenge_notice(uint8_t challenge, uint8_t page) {
    static const char *const notices[3][4][2] = {
        {
            { "RETO 01 COMPLETO", "A GUARDA EL 42" },
            { "EL 8080 SALIO", "EN ABRIL DE 1974" },
            { "FUE EL CEREBRO", "DEL ALTAIR 8800" },
            { "LA HISTORIA VIVE", "EN TUS REGISTROS" }
        },
        {
            { "RETO 02 COMPLETO", "MEMORIA 4200 OK" },
            { "EL ALTAIR 8800", "APARECIO EN 1975" },
            { "TENIA PALANCAS", "NO HABIA TECLADO" },
            { "AHORA ENTENDES", "NO ES DECORACION" }
        },
        {
            { "RETO 03 COMPLETO", "SERIE EN 4300 OK" },
            { "64 KILOBYTES ERA", "UNA GRAN MEMORIA" },
            { "HOY UNA FOTO USA", "MAS ESPACIO" },
            { "PERO 64K BASTABA", "PARA UN MUNDO" }
        }
    };

    if (page > 3) page = 3;
    lcd_print_line(0, notices[challenge - 1][page][0]);
    lcd_print_line(1, notices[challenge - 1][page][1]);
    lcd_display(true, false, false);
}

static void update_challenge_notice(uint32_t now) {
    if (!emu.cpu.halted) {
        challenge_halt_seen = false;
        challenge_notice_until = 0;
        challenge_notice_id = 0;
        return;
    }

    if (!challenge_halt_seen) {
        uint8_t challenge = completed_challenge();
        challenge_halt_seen = true;
        if (challenge) {
            challenge_notice_id = challenge;
            challenge_notice_started = now;
            challenge_notice_until = now + CHALLENGE_NOTICE_MS;
        }
    }

    if (challenge_notice_until != 0 && (int32_t)(challenge_notice_until - now) > 0) {
        uint8_t page = (now - challenge_notice_started) / CHALLENGE_NOTICE_PAGE_MS;
        show_challenge_notice(challenge_notice_id, page);
    }
}

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
        update_challenge_notice(to_ms_since_boot(get_absolute_time()));

        sleep_ms(10);
    }

    return 0;
}
