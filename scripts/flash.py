#!/usr/bin/env python3
"""Explicitly requested normal programming only; no unlock/mass erase/recovery."""
import argparse
from pathlib import Path
import os
import subprocess

parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument("mcu", choices=["F407", "F427"])
parser.add_argument("--config", choices=["Debug", "Release"], default="Debug")
parser.add_argument("--dry-run", action="store_true")
args = parser.parse_args()
root = Path(__file__).resolve().parent.parent
elf = root / "build" / args.mcu / args.config / ("Lab1_" + args.mcu + ".elf")
if not elf.is_file():
    parser.error(f"Build first: make firmware MCU={args.mcu} CONFIG={args.config}")
cfg = "SDK1_1_M FTDBG.cfg" if args.mcu == "F407" else "SDK1_1_M_F427 FTDBG.cfg"
# Escape a Tcl quoted word as well as avoiding shell evaluation.
word = str(elf)
for char in ['\\', '"', '$', '[', ']']:
    word = word.replace(char, '\\' + char)
command = [os.environ.get("OPENOCD", "openocd"), "-f", str(root / "debugger" / cfg),
           "-c", f'program "{word}" verify reset exit']
print(f"Target: {args.mcu}; firmware: {elf}", flush=True)
if args.dry_run:
    print(command)
else:
    subprocess.run(command, check=True)
