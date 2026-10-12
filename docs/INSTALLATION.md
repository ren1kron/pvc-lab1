# Installation on an Apple Silicon Mac

This guide starts with an M1 Pro MacBook and no STM32 software. It leads to the
provided projects; you do **not** need to create a new STM32 project by hand.

## A. Equipment

1. Borrow the **SDK-1.1M** with a `.407` or `.427` processor module from the lab.
2. Use its **USB 2.0 Micro-B data cable**. A charging-only cable will power a board
   but cannot communicate. A USB-A cable requires a working USB-C adapter/hub on
   the MacBook; a USB-C-to-Micro-B data cable is another option.
3. The board already contains the microcontroller module, LEDs, button, voltage
   regulators and FTDI USB programmer/debugger. You do not need an external
   ST-LINK, breadboard, LED resistors or button wires for this laboratory.
4. Install STM32CubeIDE; optionally install standalone CubeMX for `.ioc` viewing
   in IDE 2.x, and OpenOCD if the IDE's bundled OpenOCD lacks FTDI support.
5. Terminal tests also need Apple's C command-line tools and Python 3. Firmware
   builds need an ARM compiler, supplied by CubeIDE or the optional installer below.

## B. Select the IDE version deliberately

The actual development host was **arm64, macOS 26.2 (Tahoe)**. No CubeIDE GUI was
installed here, so import, graphical configuration and physical flashing remain
unverified. The independently compiled firmware does not depend on the GUI.

| Choice | Architecture / compatibility | How `.ioc` opens |
|---|---|---|
| Recommended for this host: CubeIDE 2.2.0 Apple Silicon installer | Native ARM64 installer is offered; ST lists macOS 26 support | Separate STM32CubeMX |
| Course-aligned historical CubeIDE 1.9.0 | Intel build; Rosetta 2 required on M1; macOS 26.2 operation not verified | Integrated CubeMX 6.5 |
| Other CubeIDE 1.x | Check that release's supported macOS and architecture | Usually integrated CubeMX |

The current platform facts are from ST's [download page](https://www.st.com/en/development-tools/stm32cubeide.html)
and [release notes](https://www.st.com/resource/en/release_note/rn0114-stm32cubeide-release-v200-stmicroelectronics.pdf).
Keep the STM32CubeF4 package at 1.27.1 even if the IDE is newer. A newer editor/compiler does not change
the PCB wiring, Cortex-M4 architecture, project target or FTDI hardware.

### Install CubeIDE

1. Open the official [STM32CubeIDE page](https://www.st.com/en/development-tools/stm32cubeide.html).
   Choose **Get Software**. An ST account, sign-in or email confirmation may be
   required. Choose the **macOS Apple Silicon** package (listed as
   `STM32CubeIDE-McA`), not the Intel or Windows installer.
2. Download the installer for the selected release. Open the downloaded archive
   if necessary, then open its disk image/installer and follow its installation
   prompts and license terms. Install the application in Applications or the
   location the ST installer offers. Administrative prompts belong to this
   normal installation; this project does not require security-policy changes.
3. Start STM32CubeIDE from Finder/Applications. If macOS asks whether to open a
   downloaded application, verify it came from ST and use the normal Open action.
   For a blocked signed download, consult **System Settings → Privacy & Security**.
   Do not disable Gatekeeper globally.
4. If deliberately installing the historical **Intel** version, macOS normally
   offers to install Rosetta when first launching it. Follow that Apple prompt.
   See [Apple's Rosetta instructions](https://support.apple.com/en-us/102527) and
   [ST's installation guide](https://www.st.com/resource/en/user_manual/um2563-stm32cubeide-installation-guide-stmicroelectronics.pdf).
   The native Apple Silicon IDE does not require Rosetta for its main executable;
   a separately installed Intel-only tool may still request it.
5. In the workspace selection dialog, choose a new directory such as
   `~/STM32CubeIDE/workspace-lab1`. Keep it separate from the repository.
   Workspace settings/caches are not firmware source files.
6. Confirm the workbench opens. Open **Help → About STM32CubeIDE** to check the
   version. If the old IDE cannot launch on this macOS, use the current native
   package instead; Windows is not required by the board or this project.

The course's split-volume 1.9.0 macOS download links are in the supplied Autumn
2026 DOCX. Download both parts before opening the first with a compatible archive
program. They are a course distribution route, not proof of Tahoe compatibility.

### Install the STM32F4 package when using CubeMX

The repository already contains every HAL/CMSIS file needed to **compile**.
Installing the full package is needed for viewing/regenerating configurations
with CubeMX, not for an offline Make build.

1. With CubeIDE 1.x, open **Help → Manage Embedded Software Packages**. With 2.x,
   install [standalone STM32CubeMX](https://www.st.com/en/development-tools/stm32cubemx.html)
   and use its embedded-package manager. CubeMX is separate beginning with IDE
   2.0, as explained in [ST's release announcement](https://community.st.com/stm32cubeide-mcus-28/stm32cubeide-2-0-0-released-159542).
2. Find **STM32F4**, show available/older versions, and select **1.27.1**. The
   official package page is [STM32CubeF4](https://www.st.com/en/embedded-software/stm32cubef4.html).
   Do not silently migrate to the latest pack when opening the supplied `.ioc`.
3. For the course's offline procedure, obtain **1.27.0 base** and **1.27.1 patch**
   ZIP files from its DOCX links or ST's available versions. Use **From Local…**
   to install the base, then the patch. The course specifically says to copy the
   unopened base ZIP into the configured firmware repository before installing
   the patch. Create that directory if necessary. On macOS it is commonly under
   `~/STM32Cube/Repository`; use the actual path shown by your installation.
4. Check the installed package list and the `.ioc` field
   `ProjectManager.FirmwarePackage=STM32Cube FW_F4 V1.27.1`.
5. If ST's updater is unreachable, the course recommends offline mode. In the
   1.x Preferences (on macOS often **STM32CubeIDE → Preferences/Settings**), open
   **STM32Cube → Firmware Updater**, select **Off Line Mode**, **Manual Check**,
   and **No Auto-Refresh at Application start**. Menu placement differs by version.
6. No generation is needed for this delivery. If an old pack cannot be installed
   on a new CubeMX, compile the checked-in source and inspect the `.ioc` as text;
   follow the regeneration notes before attempting an upgrade.

### FTDI debugger support

The official SDK repository states that **macOS needs no separate driver install**.
OpenOCD's FTDI backend uses libusb; neither Zadig/WinUSB nor a Windows FTDI setup
belongs on macOS. ST-LINK server software cannot substitute for the FTDI backend.

1. Try the IDE-provided OpenOCD using the supplied user-defined script.
2. If it reports that the `ftdi` adapter is unavailable, use an FTDI-enabled
   OpenOCD. The native [xPack OpenOCD 0.12.0-7 release](https://github.com/xpack-dev-tools/openocd-xpack/releases/tag/v0.12.0-7)
   was executed in this environment and parsed our scripts. Its direct
   [macOS arm64 archive](https://github.com/xpack-dev-tools/openocd-xpack/releases/download/v0.12.0-7/xpack-openocd-0.12.0-7-darwin-arm64.tar.gz)
   is available without needing to build OpenOCD. Download/extract it to a tools
   directory and run its `bin/openocd --version`. Keep its bundled libraries and
   scripts beside the executable.
3. Set the IDE OpenOCD executable path, or use the external-server instructions in
   [BUILD_AND_FLASH.md](BUILD_AND_FLASH.md). For the terminal helper:
   `export OPENOCD="/your/tools/xpack-openocd-0.12.0-7/bin/openocd"`.
4. Do not install or unload kernel extensions preemptively. If the USB interface
   is already claimed, close the program holding it and reconnect first.

## C. Install terminal tools (optional for IDE builds)

1. Open Terminal and run `xcode-select -p` and `cc --version`. If developer tools
   are absent, run `xcode-select --install` and finish Apple's dialog.
2. Check `make --version` and `python3 --version`. If Python 3 is absent, install
   it from [python.org's macOS downloads](https://www.python.org/downloads/macos/).
   Tests themselves are C; Python runs repository/ELF audits and the flash helper.
3. From the repository root, run:

   ```sh
   sh scripts/install_toolchain_macos.sh
   export PATH="$PWD/.tools/arm-gnu-toolchain-14.2.rel1-darwin-arm64-arm-none-eabi/bin:$PATH"
   arm-none-eabi-gcc --version
   make test
   make firmware MCU=F407 CONFIG=Debug
   ```

   The installer downloads Arm's native **14.2.Rel1** archive, checks its pinned
   SHA-256 and extracts only inside `.tools/`. It does not alter system PATH;
   repeat the `export` in each new shell. The same archive/compiler was used for
   the verified builds. Arm's [official toolchain page](https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads)
   also provides other host builds. CubeIDE's bundled compiler is an alternative;
   set `CROSS_COMPILE` to its `arm-none-eabi-` executable prefix.

## D. Get and import this repository

1. Clone the supplied bundle as described in [README](../README.md), or extract
   the supplied repository archive. Use a normal writable directory. Avoid
   putting it inside a cloud-synchronized workspace during a debug session.
2. Launch CubeIDE and select the workspace created above.
3. Use **File → Import → General → Existing Projects into Workspace → Next**.
4. Choose **Select root directory** and browse to the repository's root.
5. Select **Lab1_F407** for a `.407`, or **Lab1_F427** for a `.427`. You may import
   both for comparison. Leave **Copy projects into workspace** unchecked.
6. Finish. The common Core and Drivers directories appear as linked folders in
   F427. If they have red error markers, verify the two project directories are
   still siblings in the original repository layout.
7. In **Project → Properties → C/C++ Build → Settings → MCU Settings**, check
   `STM32F407VGTx` or `STM32F427VITx`. In compiler defined symbols, confirm
   `USE_HAL_DRIVER` and the matching `STM32F407xx`/`STM32F427xx`.
8. Open the project's `.ioc`. In 1.x this uses integrated CubeMX; in 2.x open it
   in the separately installed CubeMX. If asked to migrate/generate, keep the
   original configuration for initial validation. Inspect PC15 input, PD13–15
   outputs, **SYS → Debug → JTAG (5 pins)**, HSI 16 MHz, and **LSE disabled**.
9. Inspect `Core/Inc/app_config.h` and build with the steps in
   [BUILD_AND_FLASH.md](BUILD_AND_FLASH.md). Only after a successful build proceed
   to the [board connection guide](HARDWARE_SETUP.md).
