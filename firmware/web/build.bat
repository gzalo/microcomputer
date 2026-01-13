@echo off
REM Build script for web emulator using Emscripten (Windows)
REM Run after setting up Emscripten environment: emsdk_env.bat

echo Building 8080 Microcomputer Web Emulator...

REM set path to where The compiler is installed, in "$env:LocalAppData\emsdk"

set PATH=%PATH%;%LocalAppData%\emsdk\upstream\emscripten

emcc main_web.c lcd_web.c shift_register_web.c ../cpu8080.c ../memory.c ../disasm.c ../microcomputer.c ^
    -O2 ^
    -s WASM=1 ^
    -s EXPORTED_RUNTIME_METHODS="['cwrap','UTF8ToString']" ^
    -s ALLOW_MEMORY_GROWTH=1 ^
    -s MODULARIZE=0 ^
    -s EXPORT_NAME="Module" ^
    -I. ^
    -I.. ^
    -o emulator.js

if %ERRORLEVEL% NEQ 0 (
    echo Build failed!
    exit /b 1
)

echo Build complete!
echo Files generated: emulator.js, emulator.wasm
echo.
echo To run locally, start a web server:
echo   python -m http.server 8000
echo Then open http://localhost:8000
