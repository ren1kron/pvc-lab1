# Questions to prepare for the defense

| Question | Concise answer |
|---|---|
| What does Variant 3 do? | Record completed button presses as dots/dashes, confirm with yellow/red, then reproduce the sequence in green after inactivity. |
| Why only eight symbols by default? | The updated manual of 23 September 2026, printed p. 77, says up to eight. Capacity is configurable and 64 is also tested. |
| Which pins are used? | PC15 button; PD13 green; PD14/PD15 the two terminals of the red/yellow LED. |
| Is the button active high? | No. An external 3 kohm pull-up gives idle high; the switch connects to ground. |
| Why `GPIO_NOPULL`? | The SDK processor module already has the external pull-up. |
| How do red and yellow differ electrically? | Yellow: PD14 high, PD15 low. Red: PD14 low, PD15 high. Equal levels turn it off. |
| Why enable the GPIO clock? | The GPIO peripheral registers need their AHB1 clock for configuration/access. |
| What are push-pull and open-drain? | Push-pull drives high and low. Open-drain drives low or releases the line, which needs a pull-up for high. LEDs here use push-pull. |
| Does GPIO output speed set blink speed? | No. It controls edge slew capability. Software timers set the pulse durations. |
| Why use BSRR? | It sets/resets selected output bits atomically; HAL GPIO writes use it. |
| Why debounce when there is a capacitor? | Mechanical contacts still need a stable software interpretation; 30 ms unchanged level produces one accepted edge. |
| How is duration measured? | Subtract timestamps of the first samples of the subsequently confirmed press/release edges. |
| Exactly 400 ms means what? | Dash. Only durations strictly below 400 ms are dots. |
| Can playback begin while held? | No. Both a raw candidate press and a stable held press prevent timeout completion. |
| Why not `HAL_Delay`? | It would stop cooperative event processing; elapsed-time checks return immediately. |
| What clock is used? | Internal HSI 16 MHz, no PLL, all bus dividers one. SysTick supplies a nominal 1 ms tick. |
| What happens when the tick wraps? | Unsigned `now - start` still gives the correct elapsed interval across one wrap. |
| Does `__WFI` busy-wait? | No. It sleeps the core until an interrupt, normally the next SysTick. |
| What does each driver know? | Button knows electrical level/debounce/events; LED knows color wiring. Only Morse knows dot/dash classification and sequencing. |
| What if the buffer fills? | Keep the first eight; acknowledge additional releases and restart timeout but do not append them. |
| What if the user presses during playback? | Discard it. A press held across the end must be released before a fresh one can be accepted. |
| What if an update is late? | The current interval can become longer. One edge per update preserves visible pulses and gaps. |
| Why separate F407 and F427 projects? | They have different device definitions, vector tables and memory limits. One binary is not declared interchangeable. |
| Why preserve five JTAG pins? | The board's FTDI adapter uses JTAG. Reconfiguring those pins can break the debugging connection. |
| Does the IDE's “ST-LINK (OpenOCD)” label describe the hardware? | No. It selects an IDE backend; the custom OpenOCD script selects the actual FTDI adapter. |
| What was tested without a board? | Native application/driver tests, sanitizers, GPIO simulation, both MCU builds and linker/vector checks. See the verification report for actual results. |
| What still needs a board? | FTDI detection, target connection, programming/verify, actual color/timing/button behavior, reset and repeated transmissions. |

Before the demonstration, locate `Morse_Update`, `Button_Update`, `LED_On`,
`app_config.h`, and the correct linker and OpenOCD scripts. Explain one complete
dot-dash-dot sequence using the [state diagram](ARCHITECTURE.md#morse-finite-state-machine).
Run the [demonstration walkthrough](USER_GUIDE.md) with breakpoints disabled.
Do not describe simulated output as an actual hardware demonstration.
