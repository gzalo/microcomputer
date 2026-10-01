# Soluciones de las tres transmisiones

**Contiene spoilers.** Estas son soluciones posibles para los retos de [Expediente 8080](guia-fiesta-hacker.md). Todas las direcciones y los bytes de las tablas están en hexadecimal.

Para cargar una solución, detené la marcha, poné el contador de programa (`PC`) en `0000` con **STORE ADDR**, activá **AUTO INCREMENT** y escribí los bytes de la tabla, en orden, con **STORE BYTE**. La fila superior de interruptores fija el byte que se escribe. Luego poné otra vez el `PC` en `0000` y ejecutá con **SINGLE STEP** o el selector de marcha. Si la CPU quedó detenida por un `HLT` anterior, pulsá **RESET** para reanudarla; con los ejemplos opcionales habilitados, seleccioná `00` antes de hacerlo para evitar cargar uno.

El byte en una fila marcada como *operando* es parte de la instrucción anterior: no se ejecuta como una instrucción nueva. En `LXI H`, los dos bytes de la dirección van en orden **bajo, alto**. `M` significa la celda de memoria apuntada por el par de registros `HL`.

## 01 — Registro perdido

Objetivo: detener la CPU con `A = 42`.

| Dirección | Byte | Mnemónico o función del byte |
| --- | --- | --- |
| `0000` | `3E` | `MVI A,42`: carga un valor inmediato en `A`. |
| `0001` | `42` | Operando de `MVI A,42`: el valor que recibirá `A`. |
| `0002` | `76` | `HLT`: detiene la CPU. |

Secuencia completa: `3E 42 76`. Al llegar a `HLT`, `A` contiene `42`.

## 02 — Puerto sellado

Objetivo: escribir `3C` en la dirección de memoria `4200` y luego detener la CPU.

| Dirección | Byte | Mnemónico o función del byte |
| --- | --- | --- |
| `0000` | `21` | `LXI H,4200`: carga la dirección `4200` en `HL`. |
| `0001` | `00` | Operando de `LXI H,4200`: byte bajo de la dirección. |
| `0002` | `42` | Operando de `LXI H,4200`: byte alto de la dirección. |
| `0003` | `36` | `MVI M,3C`: escribe un valor inmediato en la celda apuntada por `HL`. |
| `0004` | `3C` | Operando de `MVI M,3C`: valor que se escribe en `4200`. |
| `0005` | `76` | `HLT`: detiene la CPU. |

Secuencia completa: `21 00 42 36 3C 76`. El `3C` en `0004` es un dato de `MVI M`, aunque `3C` sería el opcode de `INR A` si se ejecutara por separado. No hace falta cargar nada manualmente en `4200`: el programa lo escribe al ejecutarse.

## 03 — Huella en serie

Objetivo: escribir `10 11 12 13` en `4300 4301 4302 4303`, respectivamente, y luego detener la CPU.

| Dirección | Byte | Mnemónico o función del byte |
| --- | --- | --- |
| `0000` | `21` | `LXI H,4300`: apunta `HL` a `4300`. |
| `0001` | `00` | Operando de `LXI H,4300`: byte bajo de la dirección. |
| `0002` | `43` | Operando de `LXI H,4300`: byte alto de la dirección. |
| `0003` | `36` | `MVI M,10`: escribe en la celda apuntada por `HL` (`4300`). |
| `0004` | `10` | Operando de `MVI M,10`: dato para `4300`. |
| `0005` | `23` | `INX H`: avanza `HL` a `4301`. |
| `0006` | `36` | `MVI M,11`: escribe en `4301`. |
| `0007` | `11` | Operando de `MVI M,11`: dato para `4301`. |
| `0008` | `23` | `INX H`: avanza `HL` a `4302`. |
| `0009` | `36` | `MVI M,12`: escribe en `4302`. |
| `000A` | `12` | Operando de `MVI M,12`: dato para `4302`. |
| `000B` | `23` | `INX H`: avanza `HL` a `4303`. |
| `000C` | `36` | `MVI M,13`: escribe en `4303`. |
| `000D` | `13` | Operando de `MVI M,13`: dato para `4303`. |
| `000E` | `76` | `HLT`: detiene la CPU. |

Secuencia completa: `21 00 43 36 10 23 36 11 23 36 12 23 36 13 76`.

En los tres casos, la máquina reconoce el reto al ejecutar `HLT` y entonces borra toda la RAM. Si querés inspeccionar los bytes escritos en `4200` o `4300`–`4303`, hacelo **antes** del último paso.
