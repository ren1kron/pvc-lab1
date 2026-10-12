# ITMO ПВС — Laboratory 1, Variant 3: Morse transmitter

Complete embedded C firmware for the **SDK-1.1M** board, with separate
**STM32F407VGT6** and **STM32F427VIT6** CubeIDE projects. A debounced side button
records dots/dashes, yellow/red flashes confirm each release, and the green LED
plays the sequence once after 1.5 seconds of inactivity. All runtime drivers and
application updates are non-blocking. There is no UART, display, keypad, RTOS,
EXTI or additional timer configuration.

**Ready for physical validation; not yet flashed or tested on university hardware.**
The ARM builds and native tests are verified; the CubeIDE GUI itself was not
available on the development machine. See [verification evidence](docs/VERIFICATION.md).

## Two important findings from the official materials

1. The [updated assignment](https://disk.yandex.ru/i/KhaTar9dgFsjzQ), revision
   **23 September 2026**, specifies sequences of **up to eight presses**. Therefore
   `MORSE_CAPACITY` defaults to 8, with a tested override to 64. It also adds a
   GPIO study exercise; the included host [register simulator](docs/GPIO_SIMULATOR.md)
   implements its second option without replacing HAL in the firmware.
2. The red/yellow LED is **antiparallel**, not two independently active-high LEDs.
   The [official schematic](https://github.com/lmtspbru/SDK-1.1M/blob/master/SDK1.1M.pdf)
   pages 2–3 shows **yellow = PD14 high, PD15 low**, **red = PD14 low, PD15 high**.
   The simple color-to-pin table in the initial brief is therefore insufficient
   and has its high-side color labels reversed. The driver implements the schematic.

Full source comparison and page references: [RESEARCH.md](docs/RESEARCH.md).

## Start here

For a first session, follow [INSTALLATION.md](docs/INSTALLATION.md), then
[HARDWARE_SETUP.md](docs/HARDWARE_SETUP.md), [BUILD_AND_FLASH.md](docs/BUILD_AND_FLASH.md)
and the exact [demonstration walkthrough](docs/USER_GUIDE.md).

Identify the marking on the processor module before choosing a project:

| Module / chip | Import project directory | Eclipse project | Linker flash / normal SRAM |
|---|---|---|---|
| SDK-1.1MC.407 / STM32F407VGT6 | `firmware/` | `Lab1_F407` | 1024 KiB / 128 KiB |
| SDK-1.1MC.427 / STM32F427VIT6 | `firmware-f427/` | `Lab1_F427` | 2048 KiB / 192 KiB |

Both chips also have 64 KiB CCM and 4 KiB backup RAM, unused here. Each build uses
its own CMSIS device define, startup vector table, linker and debugger name.
**Do not interchange the resulting binaries.**

### CubeIDE

1. Keep the whole repository together. Use **File → Import → General → Existing
   Projects into Workspace**, select the repository root, and select the matching
   project. **Leave “Copy projects into workspace” unchecked**: the F427 project
   uses relative links to the common source and HAL directories.
2. Select **Project → Build Configurations → Set Active → Debug**, then
   **Project → Build Project**. HAL/CMSIS are already included; generation is not
   required. The ELF is `firmware/Debug/Lab1_F407.elf` or
   `firmware-f427/Debug/Lab1_F427.elf`.
3. Connect a **data-capable Micro-B cable to DBG USB**, check **VTGT**, then open
   the matching **Lab1_Fxxx FTDI Debug** launch configuration. Its historical
   “ST-LINK (OpenOCD)” label selects an IDE backend; the actual adapter is FTDI/JTAG.
   Check the custom script before starting. See the detailed guide for current IDE
   menus and the external OpenOCD fallback.
4. Debug/download, then **Resume** if stopped at `main`. Run mode on some IDE
   versions requires pressing the board's **RESET** button after downloading.

### Terminal

Requires `make`, a host C compiler, Python 3, and an `arm-none-eabi-` GNU toolchain.
On this Apple Silicon Mac, Arm GNU 14.2.Rel1 was verified. Optional local installer:

```sh
sh scripts/install_toolchain_macos.sh
export PATH="$PWD/.tools/arm-gnu-toolchain-14.2.rel1-darwin-arm64-arm-none-eabi/bin:$PATH"
make test
make test-sanitize
make test-capacity64
make simulate
make firmware MCU=F407 CONFIG=Debug
make firmware MCU=F427 CONFIG=Debug
make audit
python3 scripts/check_elf.py F407 build/F407/Debug/Lab1_F407.elf
```

`sh scripts/verify.sh` runs the full offline suite and all four MCU/configuration
builds. It never flashes a board. The verified results are **18 firmware tests,
7 GPIO model test groups, sanitizer and capacity-64 passes**, plus successful
F407/F427 Debug/Release builds and relocation checks.

Use `CONFIG=Release` for size optimization. Compiler paths can instead be supplied
through `CROSS_COMPILE=/path/to/bin/arm-none-eabi-`. Command-line artifacts are
under `build/<MCU>/<configuration>/`, separately from CubeIDE build directories.

Once the correct board is connected and the build exists, with OpenOCD installed:

```sh
python3 scripts/flash.py F407 --dry-run
python3 scripts/flash.py F407
# For a confirmed STM32F427VIT6 board instead:
python3 scripts/flash.py F427
```

The second/third commands perform normal programming and verification. They do
not perform automatic unlock or mass-erase recovery. `OPENOCD` can specify the
path to a particular OpenOCD executable. Choose only the command for your board.

## Timing and behavior

All constants live in [app_config.h](firmware/Core/Inc/app_config.h).

| Setting | Value |
|---|---:|
| Stable-edge debounce | 30 ms |
| Dot / dash classification | less than 400 ms / at least 400 ms |
| Yellow / red confirmation | 150 ms |
| Input inactivity after a confirmed release | 1500 ms |
| Green dot / dash | 200 ms / 600 ms |
| Green gap between symbols | 200 ms |
| Stored symbols | 8 by default |

Holding the button prevents input completion. Overflow keeps the first eight
symbols; later releases still receive classification feedback and restart the
timeout. Presses during playback, including a hold crossing its end, are ignored.
A button held at boot must be released before entering a new symbol. Rapid
feedback events replace/restart the current color flash. Pausing at a debugger
breakpoint suspends timing; demonstrate at normal execution speed.

## Repository map

```text
README.md, Makefile, LICENSE, THIRD_PARTY.md
firmware/                     Lab1_F407: .project, .cproject, .ioc, .launch, .ld
  Core/Inc/                   app_config, led, button, morse, HAL/board headers
  Core/Src/                   main, GPIO, LED/button drivers, Morse FSM, HAL glue
  Startup/                    official STM32F407 assembly vectors/reset
  Drivers/                    pinned STM32CubeF4 1.27.1 HAL/CMSIS + licenses
firmware-f427/                Lab1_F427 metadata, linker and F427 Startup/
                              Core/Drivers are relative Eclipse links
debugger/                    FTDI interface, F407/F427 target scripts
  legacy/                     manual's historical OpenOCD spellings
tests/                        native C behavior tests and fake HAL boundary
simulator/                    GPIO register model and timed dot-dash-dot demo
scripts/                      compiler installer, flash helper, audits, manifest
docs/                         guides, research, architecture, verification
build/                        ignored local build outputs and simulation CSV
```

Debug/Release directories are generated by CubeIDE. Firmware artifacts are not
committed. The original supplied course PDF and DOCX remain in `docs/`.

Additional reading: [architecture/state diagram](docs/ARCHITECTURE.md),
[defense questions](docs/DEFENSE_PREPARATION.md),
[troubleshooting](docs/TROUBLESHOOTING.md),
[hardware acceptance checklist](docs/HARDWARE_VALIDATION.md).

## Git delivery in this workspace

This environment prevents writes to the workspace's `.git` path. The deliverable
therefore includes `itmo-pvs-lab1-variant3.bundle`: a complete committed Git history
created in a separate permitted temporary repository. Clone the bundle into a
normal writable location to obtain a conventional repository:

```sh
git clone /path/to/itmo-pvs-lab1-variant3.bundle itmo-pvs-lab1-variant3
cd itmo-pvs-lab1-variant3
```

An accompanying `itmo-pvs-lab1-variant3.tar.gz` contains that repository including
its `.git` directory. The source tree here is also directly usable for builds.
