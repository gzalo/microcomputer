# Microcomputer project

An Altair/Imsai style microcomputer project based on a Raspberry Pi Pico (RP2040). Instruction set is 8080, but in the future it may be switchable between 8080, 6502, Z80, RISC-V, ARM, etc. 

Address space will be 16 bit so maximum RAM/ROM is 64 KiB. 

Extra I/O devices like keyboard and console may be added later.

[Try the web version here!](https://gzalo.github.io/microcomputer/)

For a hacker-party field guide in Spanish, including the built-in CTF challenges, see [Expediente 8080](docs/guia-fiesta-hacker.md).

To build a printable A4 dossier from that Markdown file:

```sh
python -m pip install -r docs/requirements-pdf.txt
python docs/build_guia_pdf.py
python docs/build_guia_pdf.py --landscape
python docs/build_cheatsheets.py
```

The guide outputs are `output/pdf/expediente-8080-impresion.pdf` (portrait) and `output/pdf/expediente-8080-apaisado.pdf` (landscape). The credit-card-size (85.6 × 53.98 mm) references are `output/pdf/repaso-de-binario-85.6x53.98mm.pdf` (one face) and `output/pdf/instrucciones-8080-85.6x53.98mm.pdf` (two faces). `output/pdf/tarjetas-8080-hoja-a4.pdf` places all three faces on one A4 page with gray cutting borders. The instruction faces touch along their short edge so they can be cut together and folded; the binary reference is 3 cm below them. Print at **actual size / 100%**. The instruction table remains in the [Markdown guide](docs/guia-fiesta-hacker.md); the binary reference has its own [Markdown source](docs/repaso-de-binario.md). All PDFs are image-only, with black text and a photocopy/fax filter; the cards have gray borders. The guide builder uses the repo's machine photo, a vector seal, and [transparent paper-wear scans](docs/assets/paper-wear/README.md); that asset guide includes regeneration prompts and placements.

The bundled example programs are disabled by default. RESET only resets the CPU, so programs entered with the front-panel switches stay in memory. To enable loading examples on RESET, configure the Pico firmware with `cmake -S firmware -B build -DENABLE_EXAMPLE_PROGRAMS=ON`. For the web emulator, set `ENABLE_EXAMPLE_PROGRAMS=1` when running `firmware/web/build.sh` or `firmware/web/build.bat`; its program selector appears only in an enabled build.

![Microcomputer](images/panel.png)

![Photo](images/photo_new.jpg)

## Hardware

- Inputs: 
  - 16 addr/data switches (read using two pcf8574 with pullups) 
  - 8 extra buttons/switches that are read directly:
    - 2 inputs connected to 3 position switches: stop/run slow/run fast
    - 1 input connected to a momentary push button: single step
    - 1 input connected to a momentary push button: reset (PC=0)
    - 1 input connected to a momentary push button: store to address latch
    - 1 input connected to a momentary push button: store byte
    - 1 input connected to a momentary push button: store word
    - 1 input connected to a switch: auto increment Y/N
- Outputs:
  - 16 LEDs for low and high parts (driven by 2 cascaded 74hc595) 
  - 6 for alphanumeric 16x2 LCD (low 4 bits + rs + enable)

![Schematic](images/schematic.png)

## AI

The firmware for this project was created with Claude Code (Opus 4.5 model). [I've recorded most of the prompts in here](ai_prompts.md).


