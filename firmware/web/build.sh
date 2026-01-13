#!/bin/bash
# Build script for web emulator using Emscripten
# Run with: ./build.sh (after sourcing emsdk_env.sh)

set -e

echo "Building 8080 Microcomputer Web Emulator..."

# Source files
SOURCES=(
    "main_web.c"
    "lcd_web.c"
    "shift_register_web.c"
    "../cpu8080.c"
    "../memory.c"
    "../disasm.c"
    "../microcomputer.c"
)

# Emscripten compiler flags
EMCC_FLAGS=(
    -O2
    -s WASM=1
    -s EXPORTED_RUNTIME_METHODS='["cwrap","UTF8ToString"]'
    -s ALLOW_MEMORY_GROWTH=1
    -s MODULARIZE=0
    -s EXPORT_NAME="Module"
    -I.
    -I..
)

# Build
emcc "${SOURCES[@]}" "${EMCC_FLAGS[@]}" -o emulator.js

echo "Build complete!"
echo "Files generated: emulator.js, emulator.wasm"
echo ""
echo "To run locally, start a web server:"
echo "  python -m http.server 8000"
echo "Then open http://localhost:8000"
