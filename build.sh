#!/bin/sh

set -xe

OPT=-O0
clang --target=wasm32 -nostdlib -Wl,--no-entry -Wl,--export-all -Wl,--allow-undefined ${OPT} -Ideps/common.h -Iinclude src/raster.c -o raster.wasm -DNO_STDLIB -DNO_STDIO -DTARGET_WASM -DNO_MATH
