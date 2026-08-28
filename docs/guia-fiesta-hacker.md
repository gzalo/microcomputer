La caja estaba encendida cuando la encontraron. Nadie recuerda haberla conectado. El polvo cubría los interruptores, salvo una línea limpia que iba del banco de bits a la pantalla verde.

Si llegaste hasta aquí, podés intentar hacerla responder. No hay teclado, sistema operativo ni ayuda en la memoria: solo un procesador 8080, 64 KiB y los interruptores del frente.

## Despertando la máquina

Los dieciséis interruptores forman un número binario. La fila superior es el byte bajo (bits 7 a 0) y la inferior el byte alto (bits 15 a 8). Arriba significa `1`; abajo, `0`. Ese número puede ser una dirección o un dato, según el botón que uses.

La pantalla muestra la instrucción que espera en el contador de programa (`PC`) y los bytes cercanos. Gira la llave para ver los registros: `A`, `BC`, `DE`, `HL`, `SP` y las banderas.

## Los bolsillos de la máquina

Los registros son pequeñas ubicaciones dentro del procesador. Son más rápidos que la RAM y conservan lo último que la CPU estaba usando. El panel muestra los pares como palabras de 16 bits, aunque cada letra representa un byte.

| Registro | Tamaño | Para qué sirve |
| --- | --- | --- |
| `A` | 8 bits | Acumulador: es donde terminan muchas cuentas y operaciones lógicas. La primera transmisión observa este valor al detenerse. |
| `B`, `C`, `D`, `E` | 8 bits c/u | Registros de propósito general. También aparecen agrupados como `BC` y `DE`, pares de 16 bits útiles para contar o guardar direcciones. |
| `H`, `L` | 8 bits c/u | Juntos forman `HL`, el puntero de memoria más importante. En los mnemónicos, `M` significa “el byte en la dirección `HL`”. |
| `PC` | 16 bits | Contador de programa: señala el próximo byte que se ejecutará. **STORE ADDR** lo modifica directamente. |
| `SP` | 16 bits | Puntero de pila: marca dónde se guardan retornos y valores temporales con `PUSH`, `POP`, `CALL` y `RET`. |
| `F` | 8 bits | Banderas. `Z` avisa que un resultado fue cero, `C` que hubo acarreo; `S`, `P` y `AC` registran signo, paridad y medio acarreo. Sirven para los saltos condicionales. |

Para escribir una secuencia en memoria, pensá en `HL` como un dedo que señala una celda: cargalo con `LXI H`, escribí mediante `MVI M, ...` y avanzalo con `INX H`. Mirar `PC` después de cada **SINGLE STEP** es la forma más simple de saber si tu programa sigue el camino que imaginaste.

| Control | Efecto |
| --- | --- |
| **RESET** | Lleva el `PC` a `0000`. Con un programa de ejemplo seleccionado, lo vuelve a cargar. |
| **Selector de marcha** | Cambia entre parada, lento y rápido. Empieza lento: algunas cosas se entienden mejor al verlas caer. |
| **SINGLE STEP** | Ejecuta una única instrucción cuando la máquina está detenida. |
| **STORE ADDR** | Copia los dieciséis interruptores al `PC`; elige dónde mirar o escribir. |
| **STORE BYTE** | Escribe el byte bajo de los interruptores en la dirección actual. |
| **STORE WORD** | Escribe los dieciséis bits en la dirección actual, primero el byte bajo. |
| **AUTO INCREMENT** | Avanza la dirección tras cada escritura. Usalo para cargar un programa byte a byte. |

Para cargar código, pará la máquina, lleva el `PC` a `0000` con **STORE ADDR**, activá **AUTO INCREMENT** y escribí cada byte con **STORE BYTE**. Después volvé a `0000` y usá la marcha lenta, rápida o **SINGLE STEP**. RESET no borra la memoria: si querés volver a empezar, sobrescribí los bytes.

## Fragmento de la tabla de instrucciones

Esta no es una referencia completa del 8080. Es lo que alguien dejó anotado atrás de una foto quemada; lo suficiente para mover datos, escribir memoria y detenerse.

| Bytes hexadecimales | Mnemónico | Lo que hace |
| --- | --- | --- |
| `00` | `NOP` | No cambia nada; avanza al siguiente byte. |
| `76` | `HLT` | Detiene el procesador. |
| `3E nn` | `MVI A, nn` | Carga el valor `nn` en el registro `A`. |
| `06 nn` / `0E nn` | `MVI B, nn` / `MVI C, nn` | Carga un valor en `B` o `C`. |
| `21 lo hi` | `LXI H, nnnn` | Carga `HL` con una dirección; los bytes van bajo, alto. |
| `36 nn` | `MVI M, nn` | Escribe `nn` en la dirección guardada en `HL`. |
| `77` | `MOV M, A` | Escribe `A` en la dirección guardada en `HL`. |
| `23` | `INX H` | Incrementa `HL` |
| `3C` | `INR A` | Incrementa `A`. |
| `C6 nn` | `ADI nn` | Suma `nn` a `A`. |
| `C3 lo hi` | `JMP nnnn` | Salta sin condición a una dirección. |
| `C2 lo hi` | `JNZ nnnn` | Salta si el resultado anterior no fue cero. |

`nn` es un byte y `lo hi` son los bytes bajo y alto de una dirección de 16 bits. Por ejemplo, una dirección escrita como `4200` se carga como `00 42` después del opcode. La máquina no interpreta texto: solo los bytes hexadecimales que dejes en memoria.

## Las tres transmisiones

Escribí tu propio programa desde `0000`, ejecutalo y pará la CPU con `HLT`. Cuando una condición se cumpla, la pantalla LCD mostrará una señal durante unos segundos. 

| Transmisión | Condición de la señal |
| --- | --- |
| **01 — Registro perdido** | Pará la CPU con `A = 0x42`. |
| **02 — Puerto sellado** | Antes de pararla, escribí `3C` en la dirección `4200`. |
| **03 — Huella en serie** | Antes de pararla, escribí `10`, `11`, `12`, `13` en las direcciones consecutivas `4300` a `4303`. |

El resultado debe nacer de las instrucciones que cargues. Cada señal recorre te da una pequeña recompensa. La casa abandonada no tiene prisa. Las máquinas antiguas solo cuentan secretos cuando alguien se toma el trabajo de escribirlos.
