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

---

Start with the key turned and also add instructions and link to the repo (https://github.com/gzalo/microcomputer) to the web version

---

When changing program-select make it so that the reset button is pressed for a bit

---

(NO) Extract the device from this photo into its own 3d isometric model. face with green lcd should be flat, face with switches should be a bit slanted like /_/. Keep bottom, lateral and top face 

---

(NO) Make the interface of the web emulator 3d (but isometric-like, not perspective projection) similar to @Gemini_Generated_Image_so7i7wso7i7wso7i.png . Have a few controls to adjust the rotation of the camera, just using css if possible

---

Given this microcomputer (which is implemented in hardware using switches and stuff), what cool game could I implement so that people can try it out in an event? I can add some additional I/O if needed

---

Anything that uses another thing as an output maybe? let me know what it can be

---

(NO) Is there a way for this to generate a black and white ntsc/pal signal using the pi pico pins? and in that case could it be mapped directly to some of the bytes of the virtual computer memory? and if that's the case, what could be some interesting simple games to run in the emulated console?

---

Could you write some documentation in spanish that show how to use the computer? It can be a bit mysterious as it's meant to be used in a hacker party/event. Also change the logic so that there's some reward when the user completes one or more challenges, like a CTF?

---

Can you make them have to write code instead of giving them the code base? And three challenge should be:
- Halt with certain value in A
- Output a certain value to a certain address
- Output consecutive values to consecutive addresses
No need to track the challenges in the UI nor the main code. Just show them a message for a few seconds when they have managed to win it! 
The guide should also mention the different valid opcodes and what they do in a small table
Make the style even more mysterious, as if you had found this in an old abandoned place and neede to make it work

---

El mensaje de los challenges debería aparecer en la pantalla, no en la web ni en su emulador, no cambies el emulador

---

Looks good, I've tuned it a bit. Can you mention a bit more about the registers and give better "prices" once people win? Like old computer trvia or similar

---

Pueden ser más largas las frases, algo más elaborado que dure minimo 10 segundos y tenga mucho más texto explicativo

---

Add a flag to disable the example programs, keep them off by default. Improve a bit more the words that appear when the user completes each challenge, they can be a bit longer and mysterious,

---

After completing a challenge, the memory should erase (and that should appear at the end of the lcd message too, letting the user know that's happened

---

Algun mensaje (distinto en cada caso) que agradezca al usuario por encontrar lo que tenía que hacer?

---

Add another md file with "solutions to each challenge", showing which bytes to write in each position (and which mnmemonics they represent)

---

Can you add a script or something for creating a printable pdf out of the guia-fiesta-hacker.md document. It needs to follow the same similar format/style as the protocol implies. Something like an old paper, printed with a typewriter manually. Needs to look misterious. Feel free to create or pull images or other content to make it look more real, like a "stamp" or something that says "by gzalo" with some latin words like harry potter or similar. If you cannot do that in one go, let me know which images I need to generate using which prompts and I'll do that manually and store them somewhere

---

Looks great, but could you make these changes:
- make it look even older. Can make some wrinkles/coffee stains
- this is meant to be printed in a black and white printer, background can be white, fonts can be pure black
- make the typography more "jittery", as if it were from a real typewriter, and an effect similar to it being passed through a fax machine and then printed
- add some quick reference binary table for nibbles (which combination of 4 keys prints makes which symbols)

---

Make the "bitacora de campo" in a new page. Also make the 4 switch binary to hex at the end of the doc (before the bitacora) and please postprocess the entire doc using that lo-fi filter that I really like. The output can be a pdf with just the processed images, it's fine, it will be for printint only

---

If you could improve it by adding more realistic coffee stains and other typical stuff that appears in older papers, replace them with images and let me know what prompts to use to generate each one of those, and where to place the images

---

Sacale la palabra "folio" antes de cada número. Y asegurate que todas las palabras (incluso mayúsculas) que necesitan tilde lo tengan. Sacá lo de "CLASFICIACION: transmision recuperada" y lo de "continuación/archivo recuperado"
La parte de "los bolsillos de la máquina" puede estar en la siguiente pagina. Y la parte de bitacora de campo no hace falta que tenga numero de oja

---

Make the  Clave binaria de bolsillo a standalone thing (remove it from the original doc), it should measure 6x9 cm. Also make another similar cheatsheet with the instructions (but keep it in the original doc), same size, can be two faces

---

Actually they should be 85.6 mm by 53.98 mm (credit card size)

---

add a gray border to them and make them render in the same page. The ones that are double sided should be touching each other. the other one can be a few cms away so I can cut it

---

Do that and call it "repaso de binario", and the numbers that go 0000 0001 should be more spaced (to suit the header)

---

Puedes hacer un intento de renderizar el expediente-8080 de forma horizontal/apaisada también? para ver como quedaría

---

Mode compiled for web mode should keep the examples enabled