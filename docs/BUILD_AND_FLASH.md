# Build, download and debug

Prerequisites: follow [INSTALLATION.md](INSTALLATION.md) and identify the board
with [HARDWARE_SETUP.md](HARDWARE_SETUP.md). Keep both project directories and the
shared source tree in their original relative locations. Import existing projects;
do not reconstruct them in a new-project wizard.

## Build in STM32CubeIDE

1. In Project Explorer, select **Lab1_F407** or **Lab1_F427** according to the chip.
2. Right-click → **Build Configurations → Set Active → Debug** (or use the
   equivalent Project menu). The supplied launch configuration expects Debug.
3. Check **Properties → C/C++ Build → Settings**:

   | Setting | F407 | F427 |
   |---|---|---|
   | MCU | STM32F407VGTx | STM32F427VITx |
   | Device preprocessor symbol | STM32F407xx | STM32F427xx |
   | Startup | startup_stm32f407xx.s | startup_stm32f427xx.s |
   | Linker | STM32F407VGTX_FLASH.ld | STM32F427VITX_FLASH.ld |
   | Core / FPU / ABI | Cortex-M4 / FPv4-SP-D16 / hard | Same |

4. Select **Project → Clean…**, select this project and clean if switching
   configuration/toolchain or resolving stale build files. Normal edits only need
   an incremental build. Then select **Project → Build Project** or the hammer.
5. Read the **Console** and **Problems** views. Success means the compiler/linker
   finish without errors and produce the project's ELF, not merely that an editor
   has no red underlines. A displayed text/data/bss size summary confirms linking.
6. Locate **Debug/Lab1_F407.elf** inside `firmware`, or
   **Debug/Lab1_F427.elf** inside `firmware-f427`. Refresh Project Explorer with F5
   if the generated folder is not yet visible.
7. **Release** is also supplied. If debugging Release, explicitly select that ELF
   and build configuration in the launch settings. Optimized stepping can skip
   source lines; use Debug for the first demonstration.

The shared source folders appear linked in F427. Copying just `firmware-f427/` to
another location breaks them; move/clone the whole repository. There are no
machine-specific absolute paths in the checked-in project metadata.

### Terminal build

From the repository root, with ARM GCC on PATH:

```sh
make test
make firmware MCU=F407 CONFIG=Debug
make firmware MCU=F427 CONFIG=Debug
make firmware MCU=F407 CONFIG=Release
make firmware MCU=F427 CONFIG=Release
python3 scripts/check_elf.py F407 build/F407/Debug/Lab1_F407.elf
python3 scripts/check_elf.py F427 build/F427/Debug/Lab1_F427.elf
```

Choose the one target you need for normal work. `make clean` removes only the
root `build/` directory, not CubeIDE's Debug/Release folders. Make creates ELF,
BIN, HEX and a linker MAP. Use ELF for debugging and the provided flash helper.

An explicit compiler prefix works without changing PATH:

```sh
make firmware MCU=F407 CROSS_COMPILE="/your/arm-toolchain/bin/arm-none-eabi-"
```

Use a toolchain installation path without spaces for this Makefile's executable
prefix. Source repository paths with spaces are checked by the relocation test.
CubeIDE can also be run with Eclipse's headless builder. This example is a
documented workflow, **not a GUI/headless IDE run performed in this environment**:

```sh
IDE_BIN="/Applications/STM32CubeIDE.app/Contents/MacOS/STM32CubeIDE"
"$IDE_BIN" -nosplash -application org.eclipse.cdt.managedbuilder.core.headlessbuild \
  -data "$HOME/STM32CubeIDE/workspace-lab1-cli" \
  -import "$PWD/firmware" -build "Lab1_F407/Debug"
```

Set `IDE_BIN` to the actual executable in your installed application bundle. Quit
any other IDE instance using that workspace. For F427, import `firmware-f427`
and build `Lab1_F427/Debug` instead. The portable Make build is the verified
terminal path if the IDE's headless launcher differs on your version.

## Select the FTDI debugger configuration

The board's programmer is an **FTDI USB/JTAG** device. It is not an onboard ST-LINK.
The university's historical CubeIDE menu label **ST-LINK (OpenOCD)** describes the
IDE backend used to launch OpenOCD with a custom script. **ST-LINK GDB Server**
is a different backend and will not communicate with this board.

The shared launch files are already included:

- `firmware/Lab1_F407 FTDI Debug.launch`
- `firmware-f427/Lab1_F427 FTDI Debug.launch`

Before the first download:

1. Plug into **DBG USB** and check **VTGT**. Close any serial terminal, competing
   debug session or VM that has captured the FTDI USB device.
2. Open **Run → Debug Configurations… → STM32 C/C++ Application** (historically
   **STM32 Cortex-M C/C++ Application**). Select the matching **Lab1_Fxxx FTDI Debug**.
   If not listed, refresh/import the project and open its `.launch` file. If
   necessary create this launch type with **New** and use the fields below.
3. In **Main**, select the correct project and **Search Project…** for its newly
   built Debug ELF. Do this after compiling so the file exists.
4. In **Debugger**, select **ST-LINK (OpenOCD)** in the historical probe/backend
   list. Select **JTAG**, not SWD, if that field remains enabled.
5. Choose **Configuration Script → User Defined**. Select the script:

   | Physical MCU | Current OpenOCD script |
   |---|---|
   | STM32F407VGT6 | `debugger/SDK1_1_M FTDBG.cfg` |
   | STM32F427VIT6 | `debugger/SDK1_1_M_F427 FTDBG.cfg` |

   The scripts automatically locate `SDK11M_FT.cfg` next to themselves. Do not
   select only that interface file: the target script is also needed. A warning
   that CubeIDE cannot interpret a custom script is expected in the manual's
   workflow; inspect the OpenOCD console to distinguish a real Tcl error.
6. The included launch resolves the script relative to the project. If your IDE
   does not expand that field, use **Browse** to the script in your local clone.
   This is a per-installation GUI adjustment, not a source-code change.
7. Choose reset/connection **Connect under reset**, when offered. Modern scripts
   explicitly configure it; older ST versions may use `CONNECT_UNDER_RESET`.
   For the original 1.9.0-era spelling, select the matching script under
   `debugger/legacy/` instead. See [research notes](RESEARCH.md) for the difference.
8. In **Startup**, enable loading the executable and symbols, and **break at main**.
   Keep SWV disabled. Save with **Apply**. The supplied launch uses GDB port 3333.
9. If the IDE's OpenOCD reports “adapter driver ftdi not found”, select an
   FTDI-enabled external executable or use the external-server route below.

## Download and run

1. Press **Debug**. The first launch may offer to switch to the Debug perspective;
   accept it. The IDE builds, starts OpenOCD, connects GDB and downloads the ELF.
2. Inspect the Console for successful target examination, programming and verify
   messages. An OpenOCD process starting is not sufficient evidence of programming.
3. If stopped at `main`, the application has not initialized yet. Press **Resume**
   (green play/F8 in the usual keymap). The application turns its LEDs off, then
   waits for user input.
4. Enter dot-dash-dot using [USER_GUIDE.md](USER_GUIDE.md). The green playback
   demonstrates firmware execution after programming.
5. To restart during a session, use the debug **Reset/Restart** action, or suspend
   and issue `monitor reset halt` in the GDB console, then Resume. To restart
   independently, press the board's front-panel **RESET** button.
6. If using **Run → Run** with the historical IDE, the manual says to press the
   physical RESET button after the download to begin execution.
7. Set a breakpoint by double-clicking the editor gutter next to a line in
   `Morse_Update` (for example where `state = MORSE_PLAYBACK`). Resume to hit it,
   then inspect `length`, `playback_index`, `sequence`, and `state`. Step Over is
   normally F6. Use only a few breakpoints: hardware breakpoint slots are limited.
8. Remove/disable breakpoints and Resume before demonstrating timings. A stopped
   CPU cannot poll the button normally; single-step observations are not timing
   measurements. Reset after an interrupted input sequence if necessary.
9. Terminate the debug session with the red square. This ends the debugger; it may
   leave firmware running. Unplug DBG USB to power down safely, then store the
   detached cable and board.

## External OpenOCD: verified alternative when IDE menus differ

An FTDI-enabled OpenOCD was verified for script parsing on this Mac. No board was
connected, so the following physical operations remain to be performed in the lab.

For a normal one-command terminal flash after the matching Make build:

```sh
python3 scripts/flash.py F407 --dry-run
python3 scripts/flash.py F407
```

Use `F427` for that chip, or `--config Release` for a Release build. The helper
passes an absolute ELF filename as a quoted Tcl word, then requests **program,
verify, reset and exit**. A nonzero process result is a programming failure.
Ordinary programming erases the necessary flash sectors. It does not invoke
chip unlock, option-byte changes or mass-erase recovery.

For an external debug server, run in Terminal from the repository root:

```sh
openocd -f "debugger/SDK1_1_M FTDBG.cfg"
# For F427 use debugger/SDK1_1_M_F427 FTDBG.cfg instead.
```

Keep it running. In CubeIDE's OpenOCD launch, **disable starting the server**
(“Start local GDB server” / “Start server” in versions that expose it), select
remote host `localhost`, port `3333`, and the matching ELF. Then launch GDB from
the IDE. There must be one server for this board, not one from Terminal and
another started automatically by CubeIDE.

Standalone GDB also works with a compatible GDB installed by the IDE/toolchain:

```sh
arm-none-eabi-gdb build/F407/Debug/Lab1_F407.elf
```

At the GDB prompt:

```text
target extended-remote localhost:3333
monitor reset halt
load
monitor reset halt
break main
continue
```

Continue again after reaching `main`. `load` writes the target flash; do not run
it against an unidentified board. Exit GDB with `disconnect` and `quit`, then
stop the Terminal OpenOCD server with Ctrl-C.

## Editing and regeneration

LED/button/Morse logic is in separate files. Calls in `main.c` are inside CubeMX
`USER CODE` regions. Both `.ioc` files enable **Keep User Code** and disable
deleting previous files. The two targets share hardware initialization because
their chosen pins and 16 MHz configuration are identical.

Compilation requires no regeneration. Before intentionally regenerating, commit
your edits and use a temporary copy of the whole repository. Set firmware pack
1.27.1, select generation of peripheral `.c/.h` pairs and JTAG five-pin debug.
CubeMX may rewrite **project metadata, startup location, linker scripts and
syscall templates**, and may not preserve the F427 project's Eclipse links. Review
the resulting diff; retain this repository's target linker/startup selection,
relative links and custom module files. Re-run `make test`, both ARM builds and
`make audit`. Do not generate one MCU's configuration over the other MCU project.

Common errors and recovery steps are in [TROUBLESHOOTING.md](TROUBLESHOOTING.md).
