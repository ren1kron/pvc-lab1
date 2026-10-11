#!/bin/sh
# Optional standalone compiler. CubeIDE users can use its bundled compiler.
set -eu
test "$(uname -s)" = Darwin && test "$(uname -m)" = arm64 || {
  echo 'This installer is for Apple Silicon macOS. Use Arm GNU downloads for other hosts.' >&2
  exit 1
}
root=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
name=arm-gnu-toolchain-14.2.rel1-darwin-arm64-arm-none-eabi
archive="$root/.tools/$name.tar.xz"
mkdir -p "$root/.tools"
if [ ! -f "$archive" ]; then
  curl -fL --retry 2 "https://developer.arm.com/-/media/Files/downloads/gnu/14.2.rel1/binrel/$name.tar.xz" -o "$archive"
fi
actual=$(shasum -a 256 "$archive" | cut -d ' ' -f 1)
test "$actual" = c7c78ffab9bebfce91d99d3c24da6bf4b81c01e16cf551eb2ff9f25b9e0a3818 || {
  echo 'Compiler archive checksum mismatch; nothing extracted.' >&2; exit 1;
}
if [ ! -d "$root/.tools/$name" ]; then tar -xf "$archive" -C "$root/.tools"; fi
"$root/.tools/$name/bin/arm-none-eabi-gcc" --version
echo 'Add .tools/arm-gnu-toolchain-14.2.rel1-darwin-arm64-arm-none-eabi/bin to PATH.'
