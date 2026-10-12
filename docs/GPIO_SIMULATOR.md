# GPIO simulator — updated Lab 1, part 2

The September 2026 manual, printed pp. 74–76, offers a register-driver exercise
or a GPIO hardware simulator. This repository chooses the simulator, keeping
the MCU firmware on HAL as requested. It is a separate native C program and is
not linked into either target firmware.

Run from the repository root on the Mac:

```sh
make test-model
make simulate
```

`test-model` runs seven groups of register/electrical tests. `simulate` compiles
the actual `button.c`, `led.c` and `morse.c` with a small HAL adapter that reads
IDR and writes BSRR in `GpioModel`. Output is `build/gpio_trace.csv`; successful
completion prints that all six green edges were verified. No board or ARM
compiler is needed.

## Modeled interface

`simulator/gpio_model.h` exposes aligned 32-bit register accesses at RM0090
offsets. Use `GpioModel_Write/Read`, rather than directly editing `reg`, to retain
read-only/write-only and configuration-lock behavior.

| Offset | Register | Modeled behavior |
|---|---|---|
| 0x00 | MODER | Input, output, alternate or analog mode per pin |
| 0x04 | OTYPER | Push-pull drive or open-drain low/released |
| 0x08 | OSPEEDR | Stored configuration, including lock behavior |
| 0x0C | PUPDR | Internal pulls in digital modes; disabled in analog |
| 0x10 | IDR | Read-only resolved pin levels; analog input buffer reads zero |
| 0x14 | ODR | 16-bit output latch |
| 0x18 | BSRR | Atomic low-half set / high-half reset; set wins if both selected; reads zero |
| 0x1C | LCKR | 1/0/1 key sequence with unchanged pin mask, then read; freezes selected configuration fields until reset |
| 0x20/0x24 | AFRL/AFRH | Select an externally supplied AF source per pin |

Port A/B reset defaults preserve the debug pin configuration specified by
[RM0090 Rev 21, chapter 8](https://www.stmcu.jp/wp/wp-content/uploads/2017/08/RM0090_Rev21.pdf);
other modeled ports start with zero configuration. Reserved bits in the 16-bit
ODR/OTYPER are masked. IDR writes and unaligned/out-of-range writes return false.
The lock leaves ODR/BSRR writable, matching the distinction between configuration
and output data.

`GpioModel_Drive` supplies external strong levels or high impedance, and
`GpioModel_Pulls` supplies external resistors. `GpioModel_Alternate` supplies an
AF signal without simulating the whole peripheral. `GpioModel_Pads` resolves
output, external drive and pulls. Strong drive overrides pulls; the modeled
external resistor overrides the weaker internal pull. Opposing strong drivers
set the contention mask and produce a deterministic zero. Undriven pins without
pulls set the floating mask and produce zero. Neither zero is a prediction of
an actual floating or electrically contended voltage.

## Demonstration trace

The scripted physical button edges occur at 100/200 ms (dot), 400/900 ms (dash),
and 1100/1200 ms (dot). Release confirmation occurs 30 ms later. The adapter
models the external PC15 pull-up and the antiparallel LED truth table.

| Time, ms | Checked result |
|---:|---|
| 230 | Yellow confirmation |
| 930 | Red confirmation |
| 1230 | Yellow confirmation; final debounced release |
| 2730 | Green on: first dot |
| 2930 | Green off: first gap |
| 3130 | Green on: dash |
| 3730 | Green off: second gap |
| 3930 | Green on: last dot |
| 4130 | Green off; IDLE |

CSV columns are `ms,operation,port,offset,value,pads,green,red,yellow,state,length`.
State values are 0 IDLE, 1 INPUT, 2 PLAYBACK. Rows include register reads/writes,
external button changes and explicit green edges. The assertion checks use the
resolved pads; the trace also exposes intermediate individual HAL writes.
Change the `inputs` schedule in `simulator/morse_demo.c` for experiments and
update the expected-edge assertions for the new scenario.

## Scope and limits

This is a deterministic digital peripheral model with a scheduled environment,
not a full STM32 instruction-set or circuit emulator. It does not simulate RCC
clock gating, bus latency, analog voltages, contact physics, RC settling, current,
output slew from OSPEEDR, peripheral internals, bit-band aliases or byte/halfword
bus accesses. For invalid pull encodings and competing external pulls its simple
resolution is defined by the code, not an electrical guarantee. Configuration
lock acceptance is modeled through the documented word-access key sequence.

The demo explicitly initializes the modeled registers to the firmware's settings;
it does not execute `MX_GPIO_Init()` or ST's binary HAL. The native unit tests,
source/configuration audit and real ARM build complement this boundary. None
of these replace [physical validation](HARDWARE_VALIDATION.md).
