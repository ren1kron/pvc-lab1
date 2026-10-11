# Laboratory 1 Variant 3 implementation plan

Goal: an independently buildable HAL firmware and student guide for SDK-1.1MC.407
and SDK-1.1MC.427, plus native C tests and the 2026 GPIO simulation exercise.
Spec: the user's full assignment and the supplied `lab_task_text.pdf`, revision
23 September 2026, section 5.1. The user explicitly requests uninterrupted execution.

Design decisions:
- Primary CubeIDE project `firmware` (F407VG); companion `firmware-f427` (F427VI)
  links common Core and Drivers directories using relative Eclipse links.
- Each project owns its .ioc, startup, linker, project metadata and launch file.
- HSI 16 MHz, all bus dividers 1, SysTick 1 ms, no PLL, HSE or LSE.
- PC15 input, no internal pull (external 3 kohm pull-up); PD13/14/15 push-pull low
  speed. Preserve all five JTAG pins. Actual LED polarity follows the schematic:
  green PD13 high; yellow PD14 high/PD15 low; red PD14 low/PD15 high.
- Generic LED and debounced button drivers. Application has IDLE/INPUT/PLAYBACK.
- 30 ms stable-edge debounce; threshold 400 ms; 200/600 ms green pulses;
  200 ms gaps; 1500 ms inactivity; 150 ms feedback. Default capacity 8 as required
  by the updated manual; configurable through 64 and beyond within a safe limit.
- Reject extra symbols, preserve first entries, restart inactivity on any release.
- Suppress playback input including a press spanning its end; require a stable
  release before accepting a new press. Boot with a held button behaves likewise.
- unsigned elapsed subtraction; schedule from actual LED transitions to preserve
  visible pulse/gap lengths when a polling call is late.
- No dynamic allocation, UART, display, I2C, EXTI or extra hardware timers.

## Tasks (execute in order)

- [ ] 1. Research: read both manuals, schematic, examples, ST manuals and tools.
  Record sources, contradictions, exact vendor versions and host limitations.
- [ ] 2. Build foundation: vendor the minimal STM32CubeF4 1.27.1 HAL/CMSIS subset,
  retain licenses and hashes; initialize GPIO/clocks and startup; supply two IDE
  configurations and portable Make build. Build both empty firmware targets.
- [ ] 3. Drivers: write native failing tests for electrical LED truth table,
  active-low button, 30 ms stable debounce, durations, repeated get, held startup,
  rollover. Implement `LED_On/Off/AllOff`, `Button_Init/Update/GetEvent`,
  `Button_IsActive`, `Button_SuppressUntilReleased`, then make tests pass.
- [ ] 4. Morse: write failing tests for dot/dash boundaries, order, feedback,
  timeout/held button, exact pulse/gap boundaries, buffer capacity, repeated
  sequences, rollover, and playback input spanning completion. Implement the FSM
  and run the complete suite using the real drivers with a fake HAL boundary.
- [ ] 5. New manual part 2: implement and test a standalone digital GPIO register
  model (MODER, OTYPER, OSPEEDR, PUPDR, IDR, ODR, BSRR, LCKR, AFR), externally
  scheduled inputs and a CSV trace using the actual Morse application.
- [ ] 6. Documentation: installation, hardware, import/build/flash, usage,
  troubleshooting, architecture, defense, research and explicit hardware checklist.
- [ ] 7. Final audit: host tests + sanitizers, both ARM builds, linker/vector/memory
  checks, dependency hashes, relative paths, no runtime delays, metadata checks,
  independent code review, Git commit and clean-state check.

## Review focus

Electrical color reversal; input starting just before timeout; held button crossing
playback end; large tick rollover; shared project paths and per-target vector tables.

## Execution record

Research confirmed updated manual identity by SHA-256 against Yandex public API.
The blank workspace has no existing Git history; work proceeds in the requested
directory. No worktree is needed to isolate an existing implementation.
