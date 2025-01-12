#!/bin/sh

set -xe

OPT=-O2
clang --target=wasm32 -flto -ffast-math -nostdlib -Wl,--no-entry -Wl,--export-all -Wl,--allow-undefined ${OPT} -Ideps/common.h -Iinclude src/raster.c -o raster.wasm -DNO_STDLIB -DNO_STDIO -DTARGET_WASM -DNO_MATH -fvectorize

# wasm2wat raster.wasm -o raster.wat
