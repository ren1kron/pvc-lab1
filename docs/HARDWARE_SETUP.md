# SDK-1.1M connection and electrical setup

Use the `.407` or `.427` board supplied by the laboratory. The STM32 is on a
replaceable processor module. Read **STM32F407VGT6** or **STM32F427VIT6** on the
large square MCU/module label through the protective cover, or ask the instructor
to identify it. Do not remove a powered module. The USB debugger's VID/PID does
not identify which STM32 is installed.

## Connect

1. Place the board flat on the bench. Use the lab's normal switch/jumper settings.
   No peripheral-routing switches or extra connections are required for these
   directly wired LEDs and button. The MCU must boot its internal flash (normal
   BOOT0-low setup). If a board has been changed, ask staff to restore its normal
   configuration; do not guess jumper positions on an unfamiliar revision.
2. Locate **DBG USB**, connector **XS8** on the carrier, shown in the side-panel
   view (2026 manual figure 36, printed p. 48) and front-panel drawing (figure 37).
   Identify the printed label, not merely the connector shape.
3. Connect the Micro-B end of a USB data cable to **DBG USB**. Connect the other
   end through the USB-C adapter/hub to the MacBook.
4. Check the **VTGT** indicator on the face of the board. It indicates target
   power. This connector normally supplies the power needed for this exercise;
   no separate 9–36 V supply is needed for the documented USB-powered setup.
5. In macOS **System Information → USB**, check for the board's FTDI device. It
   may appear as **Dual RS232** or **SDK 1.1M Debugger**, VID **0x0403**, PID
   **0x6010**. Terminal alternative: `system_profiler SPUSBDataType`.
6. Build/select the exact MCU project before starting a programming session.

The regular **USB** connector on the processor module connects to the STM32 USB
peripheral. It is different from the carrier's **DBG USB**, which connects to the
integrated FTDI programmer. This firmware does not implement a USB application.
Using the regular USB connector can produce power without a working debugger.

## Controls used

The **side panel / edge of the processor module** contains the user button and
the green and two-color indicators (the schematic calls them SB1, VD6 “РАБОТА”
and VD7 “РЕЖИМ”). Use this user button, not a key on the 12-key matrix.
The separate **RESET** button on the front panel is SB13 on the carrier; it
restarts the MCU and clears the sequence. Orientation depends on how the stand is
placed, so use the labels and the manual's figures instead of “left/right”.

| Connection | Firmware setup | Behavior |
|---|---|---|
| PC15 / nBTN | Input, no internal pull | Released high; pressed low; external R11 pull-up |
| PD13 / GLC | Push-pull output, low speed | High = green on |
| PD14 / YARC | Push-pull output, low speed | High with PD15 low = yellow |
| PD15 / YCRA | Push-pull output, low speed | High with PD14 low = red |

All output latches are cleared **before** enabling the output drivers. Both color
pins low means off. `LED_On` sets the opposite color's drive low before selecting
the desired color. Green is independent of the shared two-color LED. The schematic
includes a bias resistor for green; a brief glow during reset is not a Morse pulse.

GPIOC and GPIOD clocks are enabled on AHB1. PC15 remains digital: MCU LSE is off.
PA13, PA14, PA15, PB3 and PB4 retain the reset JTAG functions; no application
driver writes these pins. The FTDI link is **JTAG**, not SWD.

After flashing, follow [USER_GUIDE.md](USER_GUIDE.md). Terminate the debug session
when finished, unplug the USB cable to remove power, then detach the cable from
the board before putting it back in its box. Do not carry the powered board to
the instructor; demonstrate it on the bench as the course document requests.

Sources: supplied 2026 manual chapter 3 and sections 4.1–4.2; the
[official schematic, pp. 2–3](https://github.com/lmtspbru/SDK-1.1M/blob/master/SDK1.1M.pdf).
