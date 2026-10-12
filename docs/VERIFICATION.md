# Verification report

Verification date: **12 October 2026**. Host: Apple Silicon arm64, macOS 26.2,
Apple Clang 17.0.0. ARM compiler: native Arm GNU **14.2.Rel1**, GCC 14.2.1
20241119. Dependencies: STM32CubeF4 **1.27.1**, HAL 1.8.1, CMSIS F4 2.6.8.

**The firmware has not been flashed or run on a physical SDK-1.1M.** No suitable
USB debugger was available. CubeIDE was not installed on this host, so its GUI
import/build/launch and opening the `.ioc` in CubeMX remain unexecuted. Project
metadata follows the official SDK example, with static consistency checks; the
complete independent ARM GCC builds below were actually performed.

## Executed build and test results

All four builds succeeded. Make compiles application and HAL with
`-Wall -Wextra -Werror` (vendor-only unused-parameter suppression for an ST source
function). No compiler/linker warnings were emitted in the final build logs.
Every build produces target-specific `.elf`, `.bin`, `.hex` and `.map` files.

| Target | Configuration | Flash load | Normal SRAM including 2 KiB stack reserve | Available flash / SRAM |
|---|---|---:|---:|---|
| STM32F407VGT6 | Debug, `-Og` | 5,452 B | 2,144 B | 1,048,576 / 131,072 B |
| STM32F407VGT6 | Release, `-Os` | 4,984 B | 2,144 B | 1,048,576 / 131,072 B |
| STM32F427VIT6 | Debug, `-Og` | 5,516 B | 2,144 B | 2,097,152 / 196,608 B |
| STM32F427VIT6 | Release, `-Os` | 5,044 B | 2,144 B | 2,097,152 / 196,608 B |

These are linker load/region sizes, not ELF file sizes (which contain debug
information). GNU `size` includes flash-resident init/fini arrays in its data
column, so adding that column to bss is not an exact normal-SRAM calculation.
The normal SRAM allocation includes 96 B for static storage/alignment plus the
2,048 B reservation. CCM/backup RAM are unused. No claim about measured maximum
hardware stack depth is made.

| Check | Observed result |
|---|---|
| `make test` | 18 application/driver tests and 7 GPIO model groups passed |
| `make test-sanitize` | Same 18 + 7 passed with AddressSanitizer and UndefinedBehaviorSanitizer; no findings |
| `make test-capacity64` | All 18 application/driver tests passed with capacity 64 |
| `make simulate` | Dot-dash-dot trace generated; all 6 green edges and feedback colors matched assertions |
| ARM cross-compilation | Both MCUs, Debug and Release succeeded |
| ELF validation | All four: correct ARM vectors, reset entry, stack top and memory ranges |
| `make audit` | 143 vendor-file hashes and both projects' metadata/path/source/documentation checks passed |
| Modern FTDI scripts | Both targets parsed in native xPack OpenOCD 0.12.0-7; effective connect-under-reset flags checked |
| Historical FTDI scripts | Both parsed in that OpenOCD; expected deprecated-spelling notices, no parsing errors |
| Flash helper | Both targets passed `--dry-run`; no flash command executed against hardware |
| Relocated clean source | Full offline verification, including four fresh builds, passed from a different directory containing spaces |
| Local compiler installer | Cached official archive checksum, extraction into `.tools`, and native GCC execution passed |

The compiler archive was downloaded from Arm's official server and independently
used before testing the installer's cached-archive path. No global compiler or
macOS security setting was changed. Build/tool outputs are excluded from Git.

## Coverage of the requested behavior

The host tests compile the real LED/button/Morse sources. A fake HAL supplies
electrical input and records outputs; tests advance `uint32_t` milliseconds
explicitly. They do not wait for real time and require no physical board.

| Requirement / edge case | Test evidence |
|---|---|
| Dot/dash classification | Short press; 399 ms dot and exactly 400 ms dash boundary |
| Correct feedback color | Actual driver PD14/PD15 truth table plus Morse feedback assertions |
| Debounce / one event | Bouncing press/release, sub-debounce glitches, repeated event consumption |
| Press duration / startup | Debounce timing, held-at-boot suppression, cancellation/rearm |
| Sequence order | Dot-dash-dot pulse order and transition boundaries |
| End of input | Empty input, last-release timeout, held button, candidate press just before timeout |
| Pulses and visible pauses | Exact 200/600 ms pulses, 200 ms gaps, delayed-poll gap preservation |
| State transitions | IDLE → INPUT → PLAYBACK → IDLE, one playback only, repeated transmissions |
| Static capacity | Overflow retains first entries; no out-of-bounds access under sanitizers |
| Tick rollover | Button duration plus feedback, timeout, pulse and gap timers across wrap |
| Input during playback | Completed presses discarded; held-across-end ignored; immediate fresh press accepted |
| GPIO exercise | Reset modes, pulls, IDR/ODR, BSRR priority, open drain, analog, AF, contention, lock/reset |

A separate firmware review identified a rearming corner case at the end of
playback. A regression test first reproduced the discarded fresh press; the
corrected implementation passes it. The review also identified the need to use
stock OpenOCD's explicit under-reset flags rather than relying solely on ST's
`CONNECT_UNDER_RESET` variable. The modern scripts include that correction.

## Reproduce the complete offline check

With ARM GCC on PATH, run from the repository root:

```sh
sh scripts/verify.sh
```

An explicit `CROSS_COMPILE` environment prefix can select another installation.
Set `OPENOCD` to an executable path to include modern script parsing. That script
uses `noinit; shutdown`: it does not connect to USB or program anything.

The static audit checks required files, pinned vendor hashes, two project names,
Debug/Release device defines and includes, startup/linker selections, relative
Eclipse links, `.ioc` GPIO/JTAG/clock/package fields, launch script resolution,
absence of runtime delays/loops in custom drivers/application, and local
documentation links. This is a consistency check, not a substitute for running
CubeIDE itself. `check_elf.py` inspects ARM ELF headers, flash/SRAM load ranges,
vector-table sizes, reset entry and initial stack pointer.

## Hardware-dependent acceptance

All physical steps are explicitly pending in
[HARDWARE_VALIDATION.md](HARDWARE_VALIDATION.md): actual MCU marking, FTDI/JTAG
connection, flash programming/verification, observed LED colors, button behavior,
timing, repeated transmissions, debug reset and standalone power-cycle behavior.
HSI timing tolerance and any university board revision must be evaluated there.
Historical CubeIDE 1.9.0 under Rosetta on macOS 26.2 is also untested; the
[installation guide](INSTALLATION.md) distinguishes it from the current native
IDE route supported by ST's platform documentation.
