#!/bin/sh
# Offline verification. Never connects to or programs a board.
set -eu
cd -- "$(dirname -- "$0")/.."
make audit test test-sanitize test-capacity64 simulate
for target_mcu in F407 F427; do
  for build_config in Debug Release; do
    make firmware MCU="$target_mcu" CONFIG="$build_config"
    python3 scripts/check_elf.py "$target_mcu" "build/$target_mcu/$build_config/Lab1_$target_mcu.elf"
  done
  python3 scripts/flash.py "$target_mcu" --dry-run
done
if [ -n "${OPENOCD:-}" ]; then
  for cfg in 'SDK1_1_M FTDBG.cfg' 'SDK1_1_M_F427 FTDBG.cfg'; do
    "$OPENOCD" -f "debugger/$cfg" -c 'noinit; shutdown'
  done
fi
echo 'Offline verification passed. Physical validation is still required.'
