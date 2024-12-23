#!/bin/sh

set -xe

clang --target=wasm32 -nostdlib -Wl,--no-entry -Wl,--export-all -Wl,--allow-undefined -Os -Ideps/common.h -Iinclude src/raster.c -o raster.wasm -DNO_STDLIB -DNO_STDIO -DTARGET_WASM -DNO_MATH
