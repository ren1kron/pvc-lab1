#!/usr/bin/env python3
"""Check ELF load ranges, target vector length, reset entry and SRAM stack."""
import argparse
from pathlib import Path
import struct

p = argparse.ArgumentParser(description=__doc__)
p.add_argument("mcu", choices=["F407", "F427"])
p.add_argument("elf", type=Path)
a = p.parse_args()
b = a.elf.read_bytes()
assert b[:7] == b'\x7fELF\x01\x01\x01', "Expected ELF32 little-endian"
h = struct.unpack_from('<16sHHIIIIIHHHHHH', b)
assert h[2] == 40, "Not ARM"
entry, phoff, shoff = h[4:7]
phsize, phnum, shsize, shnum, shstr = h[9:14]
flash_size, ram_size, vector_size = (1024*1024,128*1024,392) if a.mcu == 'F407' else (2048*1024,192*1024,428)
flash_end = 0x08000000
ram_end = 0x20000000
for i in range(phnum):
    typ, off, va, pa, fs, ms, flags, align = struct.unpack_from('<8I', b, phoff+i*phsize)
    if typ != 1:
        continue
    if fs:
        assert 0x08000000 <= pa and pa+fs <= 0x08000000+flash_size, "Load outside flash"
        flash_end = max(flash_end, pa+fs)
    if 0x20000000 <= va:
        assert va+ms <= 0x20000000+ram_size, "SRAM overflow"
        ram_end = max(ram_end, va+ms)
sections = [struct.unpack_from('<10I', b, shoff+i*shsize) for i in range(shnum)]
strings = sections[shstr]
names = b[strings[4]:strings[4]+strings[5]]
for s in sections:
    name = names[s[0]:].split(b'\0',1)[0]
    if name == b'.isr_vector':
        assert s[3] == 0x08000000 and s[5] == vector_size, (name, s[3], s[5])
        stack, reset = struct.unpack_from('<II', b, s[4])
        assert stack == 0x20000000+ram_size and stack % 8 == 0
        assert reset & 1 and reset == entry
        assert 0x08000000 <= (reset & ~1) < flash_end
        break
else:
    raise AssertionError('Missing vector table')
print(f'{a.mcu}: valid ARM vector/entry/stack; flash load {flash_end-0x08000000} B/{flash_size}, SRAM incl. reserve {ram_end-0x20000000} B/{ram_size}')
