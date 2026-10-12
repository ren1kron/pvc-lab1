# Troubleshooting and recovery

Start with the symptom below. Keep the exact compiler/OpenOCD error text and
identified MCU when seeking help. Do not change several settings at once.

## Compilation or project import

| Symptom | Check and fix |
|---|---|
| No project found | Select the repository root in **Existing Projects into Workspace**; hidden `.project`/`.cproject` files must have survived extraction. |
| Red link markers in F427 | Keep `firmware` and `firmware-f427` as siblings; re-import without copying projects. The F427 sources and HAL live in `firmware`. |
| `stm32f4xx_hal.h` missing | Confirm `firmware/Drivers` exists and compiler include paths match the supplied metadata. No external user-specific Cube repository is required. |
| Device header/type missing | Select exactly one target symbol, `STM32F407xx` or `STM32F427xx`, with `USE_HAL_DRIVER`. Do not define both. |
| `arm-none-eabi-gcc: command not found` | Use CubeIDE's toolchain, the installer in `scripts`, or set `CROSS_COMPILE`/PATH. Apple's `cc` builds tests, not Cortex-M firmware. |
| Undefined Reset_Handler/vector symbols | Check correct startup `.s` and linker script; do not compile both startup files together. |
| Duplicate main/SystemInit/SysTick | Remove newly generated duplicates; each target builds the shared Core once and its own Startup once. |
| Flash/RAM region overflow | Check target/linker selection and buffer changes. CCM is separate memory, not contiguous with normal SRAM. |
| Vendor unused-parameter warnings under stricter flags | Make suppresses only this warning in unchanged HAL sources. Application warnings remain errors. Do not casually edit vendored HAL. |
| IDE asks to migrate the `.ioc` | Build the supplied source first. Keep F4 package 1.27.1; regenerate only in a reviewed copy. IDE 2.x needs separate CubeMX to view it. |
| Program file does not exist | Build first, then select the actual Debug ELF in the launch's Main tab. Refresh Explorer; check selected configuration. |

## USB and OpenOCD

| Symptom | Check and fix |
|---|---|
| No VTGT light | Check data cable/hub/connector seating and power. Use DBG USB; ask staff about a suspected damaged board. |
| No FTDI USB device | Check macOS System Information → USB; try a known data cable and direct adapter instead of a hub. Check macOS accessory-connection prompts. |
| “No ST-LINK detected” | The ST-LINK GDB Server backend is wrong. Use OpenOCD with the FTDI custom script. |
| Unknown `ftdi` adapter/command | Use an OpenOCD build with FTDI/libusb; xPack's native macOS package was checked. For old syntax use `debugger/legacy`. |
| Cannot find `SDK11M_FT.cfg` | Use the complete `debugger` directory. Included scripts resolve the companion file relative to themselves. |
| Cannot find `target/stm32f4x.cfg` | Keep the OpenOCD installation's scripts directory. If relocated separately, supply `-s /path/to/openocd/scripts`. |
| LIBUSB access/busy error | Stop other debug servers, terminals or VMs using the FTDI device, then reconnect. macOS does not use Linux udev rules or Windows Zadig. Do not default to sudo or remove OS drivers. |
| GDB port 3333 already in use | Terminate the previous server. If using an external server, disable the IDE's own server launch. |
| JTAG scan/target examine fails | Check target power, cable, custom script and JTAG transport. Confirm `.407`/`.427` marking. Try the supplied connect-under-reset script and the normal RESET button. |
| Colors swapped | Check physical module revision against schematic, and ensure PD14=yellow-anode/red-cathode, PD15=reverse. Do not change only one active level: both pins form a shared LED. |

On this board, FTDI channel A/interface 0 is JTAG; interface 1 is the separate
serial channel. The official macOS workflow does not require a new USB driver.
USB visibility confirms the adapter, not a live STM32 debug connection.

## Program downloads but does not run

1. Check whether execution is halted at `main` or a breakpoint. Press Resume.
2. In historical Run mode press the board's RESET button after downloading, as
   directed by the university manual.
3. Verify the ELF matches the MCU. Flash sizes and vector tables are target-specific.
4. Check normal flash boot configuration with staff if the module was previously
   repurposed. Wrong BOOT0 may allow programming yet boot a different memory.
5. Break in `main`, then after `SystemClock_Config`: `SystemCoreClock` should be
   16000000. Confirm `HAL_GetTick()` advances when running. Do not expect it to
   advance while the debugger halts the CPU.
6. Inspect `MX_GPIO_Init`, PC15 input value and LED output latches. No input means
   all LEDs remain off; there is no continuous heartbeat animation.
7. If `Error_Handler` or a fault handler is reached, inspect the stack and the
   failed initialization call. This is a fatal error state, not normal playback.
8. Reset after experimenting with breakpoints during a press. Debug halts disrupt
   the physical-input sampling timeline.

## Conservative recovery sequence

1. Terminate the current debug server and reconnect DBG USB.
2. Reconfirm power, cable, correct `.407`/`.427` target and **JTAG**.
3. Retry the modern script, which explicitly asserts SRST while connecting.
   Legacy ST scripts use their version-specific `CONNECT_UNDER_RESET` behavior.
4. If needed, coordinate a manual RESET-held connection with lab staff. The
   board's nSRST line is configured from the official FTDI layout.
5. If timing/cabling is suspected, staff may lower JTAG adapter speed; do not
   change VID/PID, electrical layout masks or TAP IDs to hide an error.
6. If option-byte protection or corrupted flash is suspected, stop and ask the
   instructor/board owner. **Mass erase, readout unprotect, option-byte changes
   and boot-jumper changes are not automatic recovery steps in this project.**
   Some protection changes erase flash and may remove another laboratory's data.

No destructive recovery operation was performed during development.
