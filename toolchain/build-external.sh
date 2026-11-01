#!/bin/sh
# Minimal external-application build using the prepared ELKS toolchain.
set -eu
TOPDIR=${ELKSFETCH_ELKS:-${TOPDIR:-}}
: "${TOPDIR:?Set TOPDIR or ELKSFETCH_ELKS to a built ELKS checkout}"
TOPDIR=$(CDPATH= cd -- "$TOPDIR" && pwd)
export TOPDIR
PATH="$TOPDIR/elks/tools/bin:$TOPDIR/cross/bin:$PATH"
export PATH
app_root=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
cd "$app_root"
command -v ia16-elf-gcc >/dev/null
command -v elf2elks >/dev/null
mkdir -p build
# Remove the old binary before compiling: failed builds cannot install it.
rm -f build/elksfetch
ia16-elf-gcc -march=i8086 -mtune=i8086 -mcmodel=small -melks-libc \
    -mno-segment-relocation-stuff -Os -Wall -Wextra \
    -fno-builtin-printf -fno-builtin-fprintf \
    -I./src -I"$TOPDIR/include" -I"$TOPDIR/libc/include" -I"$TOPDIR/elks/include" \
    -maout-heap=4096 -maout-stack=2048 \
    -Wl,-Map,build/elksfetch.map \
    src/main.c src/render.c platform/elks.c platform/network.c -o build/elksfetch
printf '%s\n' 'ELKSfetch built: build/elksfetch'
