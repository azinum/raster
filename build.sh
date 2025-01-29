#!/bin/sh

set -xe

OPT=-O3
clang --target=wasm32 -flto -ffast-math -nostdlib -Wl,--no-entry -Wl,--export-all -Wl,--allow-undefined ${OPT} -Iinclude -Ideps/common.h src/main.c -o raster.wasm -DNO_STDLIB -DNO_STDIO -DTARGET_WASM -DNO_MATH -fvectorize

clang ${OPT} -Iinclude -Ideps/common.h src/main.c -o raster `pkg-config --libs --cflags sdl2` -lm -fvectorize -march=native -DNO_TEXTURES

# wasm2wat raster.wasm -o raster.wat
