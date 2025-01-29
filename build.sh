#!/bin/bash

set -xe

OPT=-O2
clang \
	${OPT} \
	--target=wasm32 \
	-fvectorize \
	-flto \
	-ffast-math \
	-nostdlib \
	-Wl,--no-entry \
	-Wl,--export-all \
	-Wl,--allow-undefined \
	-Iinclude \
	-Ideps/common.h \
	src/main.c \
	-o raster.wasm \
	-DNO_STDLIB \
	-DNO_STDIO \
	-DTARGET_WASM \
	-DNO_MATH \
	-DNO_OMP

clang \
	${OPT} \
	-fvectorize \
	-flto \
	-ffast-math \
	-Iinclude \
	-Ideps/common.h \
	src/main.c \
	-o raster \
	`pkg-config --libs --cflags sdl2` \
	-lm \
	-DNO_OMP && \
	strip raster

# wasm2wat raster.wasm -o raster.wat
