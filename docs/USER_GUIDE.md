# Demonstrate Variant 3

Use the side-panel user button beside the module LEDs. The front-panel RESET
button restarts the program; the matrix keys are unused.

1. Flash the correct target, Resume execution, then press **RESET**. Release the
   user button. The three controlled colors settle off after initialization.
2. Press the user button for about **0.1–0.2 s**, then release. After its 30 ms
   release debounce, **yellow flashes for 150 ms**: a dot was detected.
3. Wait about **0.3 s**, then press for about **0.6 s** and release.
   **Red flashes for 150 ms**: a dash was detected. A duration of exactly 400 ms
   belongs to the dash class, so avoid that boundary during demonstration.
4. Wait about **0.3 s**, then press for about **0.1–0.2 s** and release.
   Observe another **yellow** flash. Keep gaps between entered symbols well below
   1.5 seconds; holding a press does not trigger completion.
5. Stop touching the button. **1.5 s after the final debounced release** the green
   indicator begins. Relative to physical release this is approximately **1.53 s**.
6. Observe this timeline, starting at the first green rising edge:

   | Time | Green LED |
   |---|---|
   | 0–200 ms | On: dot |
   | 200–400 ms | Off: gap |
   | 400–1000 ms | On: dash |
   | 1000–1200 ms | Off: gap |
   | 1200–1400 ms | On: dot |
   | At 1400 ms | Off; ready for the next sequence |

7. Enter a second sequence, such as two dashes, and observe another single
   playback. No reset is needed between sequences.

The assignment transmits entered **symbols**, not decoded letters. There is no
UART text output, alphabet conversion, word separator or separate Send button.

## Edge cases to explain to the instructor

- A press shorter than the 30 ms stable debounce requirement is treated as noise.
- Press duration is measured between the first samples of the finally stable
  press/release levels; debounce does not add 30 ms to one end only.
- While the button is held, playback waits. Yellow/red feedback expiry continues
  to work while another press is held, proving updates are independent.
- The first **eight** completed presses are stored (updated 2026 requirement).
  Further releases still flash their detected color, but are not stored. Stop
  input normally; the first eight play in order after the usual timeout.
- Presses during playback are ignored. If held across its end, release and press
  again. A genuinely new press after playback is accepted immediately.
- A button held while resetting is ignored until released; this avoids a phantom
  first symbol at boot.
- A quick new confirmation replaces the current flash and restarts its 150 ms
  duration. For distinct visible confirmations leave about 0.3 s between presses.
- Settings are in `app_config.h`; thresholds are engineering choices. The ratio
  of dash to dot playback is 3:1. The exact physical durations depend on HSI
  accuracy and polling latency and must be checked on the actual board.

## Expected evidence for the laboratory

Before asking for a demonstration, have the program loaded and running. Show the
dot-dash-dot behavior, repeatability, and your source modules. Explain the state
diagram in [ARCHITECTURE.md](ARCHITECTURE.md). For the new manual's GPIO study,
run `make test-model` and `make simulate`, then inspect `build/gpio_trace.csv` using
[GPIO_SIMULATOR.md](GPIO_SIMULATOR.md).

Record **actual** observations only after performing the hardware experiment.
The supplied verification report contains host/build evidence, not fabricated
laboratory results. Use [HARDWARE_VALIDATION.md](HARDWARE_VALIDATION.md) to record
the remaining checks.
