# Verified sources and decisions

Research performed 11–12 October 2026. Source content, source-derived conclusions,
implementation choices and physical validation are distinguished below.

## University materials

- [2022 manual](https://books.ifmo.ru/file/pdf/3031.pdf): sections 1.1/1.2;
  chapter 3; 4.1, 4.2, 4.4, 4.8; and 5.1, Variant 3. Its printed pp. 7–9 show
  GPIO connections; pp. 53–57 show IDE/debug setup; pp. 59–60 discuss failures.
- [Updated manual](https://disk.yandex.ru/i/KhaTar9dgFsjzQ): available locally as
  [lab_task_text.pdf](lab_task_text.pdf). Its title page says 23.09.2026.
  Yandex's public API identifies `SDK-1.1M_26092301.pdf`, 3,695,300 bytes,
  SHA-256 `7ae82dff438b44c67fff20aa4ee9388693ed4c9c041edf2238ddf8dd5d2d9a81`.
  The local PDF matches this hash exactly. Browser preview failed, but the
  public metadata and local full text were accessible.
- The supplied DOCX named `Инфо - Проектирование вычислительных систем (лаб) - 4 курс - осень 2026.docx`
  recommends CubeIDE 1.x, provides 1.9.0 installers, and describes installation of
  STM32CubeF4 1.27.0 followed by the 1.27.1 patch. Its hyperlink for the lab manual
  points to the Yandex resource above.

The newer manual has an extra cover page: its **PDF page number is printed page
number + 1**. Section numbering also changed: 4.4 is installation, 4.5 is project
creation, and the old 4.8 troubleshooting content has moved to 4.10 (4.9 now links the schematic/examples).

| Area | 2022 text / original brief | 2026 update and implementation |
|---|---|---|
| Variant 3 capacity | Unspecified / suggested 64 | Printed p. 77: up to 8; default 8, bounded and configurable |
| Generic drivers | Non-blocking and reusable | Explicitly requires software debounce and cooperative updates |
| Interrupts/timers | General background | Printed p. 74: no interrupts or extra timers except SysTick; firmware complies |
| GPIO study | Understand HAL and GPIO | Printed pp. 74–76: an additional part, own GPIO driver **or** hardware simulator; this project supplies the simulator option |
| Tools | CubeIDE 1.7 example | Course recommends 1.x/1.9.0 and F4 pack 1.27.1; those metadata conventions and pack are retained |

The simulator paragraph mentions `STM32F427VGT6`, whereas the university board
chapter identifies the `.427` module as **STM32F427VIT6**. The simulator models the
shared F4 GPIO peripheral; the actual F427 firmware targets the VI part.

## Schematic and examples

[Official repository](https://github.com/lmtspbru/SDK-1.1M), tree
`bd8cba8f4bd5fc2d1460e6ad4da7b7dd13d9e14d`, includes
`SDK1.1M.pdf` (2021 schematic), `SDK_LED`, `SDK_EXTI` and debugger examples.
The LED project's `.ioc` selects STM32F407VGT6 and STM32CubeF4 1.27.1, making
F407 the primary target here. Its copied `SDK11M.cfg` happens to name F427;
that mismatch is not propagated into this project.

Schematic **PDF p. 2** is processor-module sheet 1, **PDF p. 3** sheet 2:

| Signal | Traceable connection | Electrical interpretation |
|---|---|---|
| Button | SB1 → R12 330 ohm → nBTN → DD4 pin 9 / PC15 | R11 3 kohm pulls toward 3.3 V; pressed switch connects to GND; C7 0.1 µF is present. Active low; internal pull unnecessary. |
| Green | DD4 pin 60 / PD13 → GLC → R13 330 ohm → VD6 anode; cathode GND | GPIO high turns on. R16 1 kohm also biases its anode from 3.3 V, so it can glow before GPIO setup. Low turns it off. |
| Two-color | DD4 pin 61 / PD14 → YARC; DD4 pin 62 / PD15 → YCRA, via R14 to the other terminal | YARC means yellow anode/red cathode; YCRA the reverse. Color depends on the voltage difference. |

| PD14 (YARC) | PD15 (YCRA) | Result |
|---:|---:|---|
| 0 | 0 | Off (initial state used here) |
| 1 | 0 | Yellow |
| 0 | 1 | Red |
| 1 | 1 | Off |

The pin numbers from the brief are retained; its assumption of a separate
active-high PD14 red LED and PD15 yellow LED is corrected. A physical check of
actual module color remains in the acceptance checklist in case of board revision.

The examples demonstrate 25 MHz HSE and a PLL, but the assignment does not require
maximum CPU speed. This project chooses HSI 16 MHz, no PLL, AHB/APB dividers 1.
The 32.768 kHz crystal shown on the carrier serves its separate RTC; this firmware
does not enable MCU LSE, whose alternate pin would conflict with PC15.

## ST and tool documentation

- [RM0090](https://www.st.com/resource/en/reference_manual/dm00031020.pdf),
  read as Rev 21 using ST's [Japanese site's English copy](https://www.stmcu.jp/wp/wp-content/uploads/2017/08/RM0090_Rev21.pdf)
  after st.com downloads failed. Chapter 8, pp. 273–291: GPIO modes, pulls,
  output latch, atomic BSRR, configuration lock and alternate-function registers.
  RCC chapters 6/7 cover HSI, reset clock configuration and AHB1 GPIO clock gates.
- [F407 datasheet DS8626](https://www.st.com/resource/en/datasheet/stm32f407vg.pdf)
  and [F427 datasheet DS9405](https://www.st.com/resource/en/datasheet/stm32f427vi.pdf):
  package, memory map, HSI and pin alternate functions. The linker uses only
  contiguous normal SRAM; CCM is not appended to that address range.
- [STM32CubeF4 v1.27.1](https://github.com/STMicroelectronics/STM32CubeF4/tree/v1.27.1):
  real HAL 1.8.1 and CMSIS F4 2.6.8 sources/startup files, unchanged and hashed.
- [STM32CubeIDE product/download page](https://www.st.com/en/development-tools/stm32cubeide.html)
  and [RN0114](https://www.st.com/resource/en/release_note/rn0114-stm32cubeide-release-v200-stmicroelectronics.pdf):
  current release checked is 2.2.0; an Apple Silicon ARM64 installer exists;
  the release notes list macOS 15 and 26. This matches the host's macOS 26.2
  better than an unverified 1.9.0 installation.
- [UM2563 installation guide](https://www.st.com/resource/en/user_manual/um2563-stm32cubeide-installation-guide-stmicroelectronics.pdf)
  documents Rosetta installation for the Intel build on M1. That does not establish
  CubeIDE 1.9.0 support on macOS 26.2. That particular historical combination was
  not installed or tested here.
- ST's [2.0 release announcement](https://community.st.com/stm32cubeide-mcus-28/stm32cubeide-2-0-0-released-159542)
  explains that CubeMX is separate starting with IDE 2.x. Existing project
  compilation does not require regenerating from `.ioc`.
- [OpenOCD FTDI options](https://openocd.org/doc-release/html/Debug-Adapter-Configuration.html)
  and [reset flags](https://openocd.org/doc-release/html/Reset-Configuration.html):
  modern spellings of the manual's FTDI commands and explicit under-reset connection.

## Debugger configuration provenance

The manual/official example specify USB VID/PID `0403:6010`, layout init
`0x0408 0x0ffb`, and `nSRST -oe 0x0800`, **JTAG**, work area `0x8000`,
`reset_config srst_only`, and `target/stm32f4x.cfg` for both MCU variants.
Their respective CHIPNAME strings are `STM32F407VGTx` and `STM32F427VITx`.
These strings name OpenOCD target objects; they do not make an F407 binary valid
on an F427 or replace probing the actual chip.

Modern scripts change command spelling and resolve the companion file relative
to the script, so working-directory changes and spaces are safe. Stock OpenOCD
does not consume ST's `CONNECT_UNDER_RESET` variable, so the modern scripts also
set `reset_config srst_only srst_nogate connect_assert_srst` after loading the
target. This is the documented stock equivalent, not a new electrical mask.
The `legacy/` directory preserves the manual's command spellings and reset setup
for historical ST OpenOCD. Stock OpenOCD's target file chooses its own initial
2 MHz JTAG rate; no speculative USB description, serial number or TAP ID is added.
