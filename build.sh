#!/bin/sh

set -xe

OPT=-Os
clang --target=wasm32 -flto -ffast-math -nostdlib -Wl,--no-entry -Wl,--export-all -Wl,--allow-undefined ${OPT} -Ideps/common.h -Iinclude src/raster.c -o raster.wasm -DNO_STDLIB -DNO_STDIO -DTARGET_WASM -DNO_MATH

# wasm2wat raster.wasm -o raster.wat
