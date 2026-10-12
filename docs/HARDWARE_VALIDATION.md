# Physical validation checklist — not yet executed

All boxes below intentionally remain unchecked. Record the actual board, date
and results during a university lab session. Automated checks are recorded
separately in [VERIFICATION.md](VERIFICATION.md).

Board/module marking: __________  MCU top marking: __________

Date/operator: __________  IDE/OpenOCD version: __________

Firmware target/configuration and Git commit: __________

- [ ] Identify STM32F407VGT6 or STM32F427VIT6 before selecting its ELF/script.
- [ ] Connect a data cable to DBG USB; verify the expected power/VTGT indication.
- [ ] Confirm the FTDI device is visible in macOS System Information → USB.
- [ ] Import the supplied project in CubeIDE and build Debug without reconstruction.
- [ ] Connect through FTDI/JTAG; record actual target identification and flash size.
- [ ] Program and verify the matching firmware; Resume after the breakpoint at main.
- [ ] Confirm idle LEDs turn off after initialization.
- [ ] A roughly 100–200 ms press produces one yellow confirmation.
- [ ] A roughly 600–800 ms press produces one red confirmation.
- [ ] Dot-dash-dot reproduces in green as 200/600/200 ms pulses with 200 ms gaps.
- [ ] Playback starts about 1.53 s after the physical final release (30 ms debounce plus 1500 ms).
- [ ] Holding the button longer than the timeout does not start playback early.
- [ ] Several taps each produce exactly one symbol; ordinary bounce adds none.
- [ ] Eight symbols are preserved in order; a ninth adds feedback but is not stored.
- [ ] A press completed during playback does not enter the next sequence.
- [ ] A press held across playback's end is ignored through its release.
- [ ] A fresh press immediately after playback works normally.
- [ ] Repeat multiple different sequences and confirm no automatic retransmission.
- [ ] Reset while idle/input/playback; application restarts cleanly.
- [ ] Power-cycle and verify standalone execution without a debugger session.
- [ ] Break at a state transition, inspect variables, remove the breakpoint and Resume.
- [ ] Terminate the debugger and disconnect safely.

If timing accuracy is required, capture PD13 with an oscilloscope/logic analyzer
using the board's permitted measurement connections and common ground. Record
measured values and instrument settings; nominal software milliseconds are not
a measurement. Confirm the actual red/yellow colors against the board revision.
Use [TROUBLESHOOTING.md](TROUBLESHOOTING.md) for failures. Do not attempt mass erase,
option-byte changes or protection removal as an automatic recovery step.

Observed results/issues: __________
