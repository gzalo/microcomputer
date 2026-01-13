**This page captures the different prompts used for creating the project**

Write firmware for the pi pico (using its sdk) to test all LEDs, all inputs and LCD
 
---

Instead of using cmake, use platformio

---
Create a cmake file for this pi pico project

---

Fix issues with this code, ensure that LED count test only runs once, and make it so that before that you can test all inputs for 10 seconds

---

The key switch is not working, and the ones related to PCF8574 neither. Can you show on the LCD whether the I2c probe worked?

---

What about the key input? why could it not be working? did you set them all up to be pull up?

---

GPIO27 is still not working. And could you make the leds turn on one at a time, not all of them? also the i2c is giving out err, could you validate the code! it may be PCF8574A

---

There seems to be an issue with the shift registers. They are 74HC595, could you sensure the timings and polarity are fine?

---

Can you swap the lowest and highest bit within a byte when sending the sr_output data?

---

Actually instead of doing the swap just rotate the 8 bit number and move the lowest to the upper position

---

Combine test led chase and input timed into a single one, make it only run if the program starts with the key off (0x100 input not set)

---

Make the LCD show the input status during the chase pattern, not after

---

When not in test mode, the thing should work like a microcomputer imsai/altair like
LCD Should show

`0123: Mov X, R0`
`00.01.10.01.12.15.00`
Underscore on second line shows cursor position, one or two bytes depending on instruction length.

Instruction set should be Z80. Address space should be 16 bit (65536 bytes)

There's inputs for stop/run slow/run fast, single step, reset (PC=0, doesn't touch ram), store to address latch, store byte, store word, autoincrement Y/N inputs.

---

The switch bytes are incorrect. When I do the ones from left to rigth (top to bottom) just one at a time I get
(top row) 0x08 0x80 0x40 0x20 0x10 0x04 0x02 0x01
(bottom row)
0x0800, 0x8000, 0x4000, 0x2000, 0x1000, 0x400, 0x200, 0x100
Can you fix that so that pcf8574_read_all reads it correctly (by swapping some bits accordingly)

---

The order of the LEDs is swapped (leftmost bit should be rightmost bit)

---

keep in mind that the existing fix (rotation) should still be there as it's wired differently

---

When over a instruction that has two bytes, have the LCD cursor underline all 4 bytes - first, second, fourth and fifth characters of the second line (sequentially every 100 ms or so), otherwise just do the same for the first 2 characters

---

The test mode should only run on startup if the key is off, otherwise if the key is off after it has started it should just show the registers in the LCD, as a debugging option

---

The screen is 20x2 so DE can fit in the first line. Also ensure LCD is refreshed after switching modes

---

Address Latch and PC can be the same thing, eg, you always see what's in PC

---

Keep it as it was (A, BC, DE) first line, (HL, SP, F) second line. 
I only meant that address latch in emulator doesn't need to exist, we can use always PC

---

When going from debug mode to emulator mode, ensure the LCD gets printed, as otherwise it remains clear

---

Any ideas of what could be useful to have in this? And can you make some example program that can be loaded in a certain part of ram to test it?

---

After loading a program show in the LCD first line which program was loaded, for 500 ms, then show the regular interface

---

The "show debug register" logic should be part of the emulator itself, and the emulation must continue when that is happening. So summarizing the key just switches whether the lcd shows one thing or the other, everything else should still work the same

---

What cool improvements could be added to the code?

---

Would it be possible to have a web version of this emulator? Using the exact code but with some extra layer for I/O

Use emscripten

---

.\build.bat
Building 8080 Microcomputer Web Emulator...
cache:INFO: generating system asset: symbol_lists/c0f289509245f4f682f936b5281d8b6db4bfe572.json... (this will be cached in "C:\Users\Gzalo\AppData\Local\emsdk\upstream\emscripten\cache\symbol_lists\c0f289509245f4f682f936b5281d8b6db4bfe572.json" for subsequent builds)
cache:INFO:  - ok
wasm-ld: error: C:\Users\Gzalo\AppData\Local\Temp\emscripten_temp_6_a5sxk1\microcomputer_4.o: undefined symbol: lcd_clear

---

Use @web/panel.png and @web/photo.jpg for design inspiration and layout for the emulator (it should be as similar as the real thing as possible)

---

@web/now.png  Make the switches and LEDs bigger (and more to the right), as they are in the photo @web/photo.jpg

---

Can you make a github action that will build and deploy the firmware/web part into github pages?