# Source provenance and licenses

- STM32 HAL/CMSIS and target startup/system files: ST's
  [STM32CubeF4 v1.27.1](https://github.com/STMicroelectronics/STM32CubeF4/tree/v1.27.1),
  commit `52757b5e33259a088509a777a9e3a5b971194c7d`.
  HAL 1.8.0, CMSIS device F4 2.6.8 (see source version macros).
  The subset is vendored; no download or Git submodule is needed to compile.
  [Manifest](scripts/vendor_manifest.json) records source paths and SHA-256 hashes.
  HAL retains its [BSD 3-clause license](firmware/Drivers/STM32F4xx_HAL_Driver/LICENSE.md);
  CMSIS core retains its [Apache 2.0 license](firmware/Drivers/CMSIS/LICENSE.txt).
  See also [ST package license](firmware/Drivers/STM32CubeF4-LICENSE.md).
- CubeIDE metadata and .ioc key conventions were adapted from the official
  [SDK_LED example](https://github.com/lmtspbru/SDK-1.1M/tree/master/SDK_LED),
  repository tree `bd8cba8f4bd5fc2d1460e6ad4da7b7dd13d9e14d`.
  Debugger electrical constants are from its README and the university manual.
  These are configuration adaptations, not an assertion that the upstream
  demonstration application implements this lab.
- The two university documents already supplied in `docs/` are retained unchanged.
  Their copyrights remain with their authors/ITMO. This project's license does
  not relicense them. The schematic is linked rather than redistributed.
- Arm GNU compiler and xPack OpenOCD were used for verification. Their binaries
  are not part of the Git repository; `.tools/` and build outputs are ignored.
