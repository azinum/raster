#!/bin/bash

set -xe

OPT=-O3
clang-18 \
	${OPT} \
	--target=wasm32 \
	-fvectorize \
	-flto \
	-ffast-math \
	-Wall \
	-Wno-missing-braces \
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
	-DNO_OMP \
	-DNO_TIMER \
	-DNO_NORMAL_BUFFER

clang \
	${OPT} \
	-fvectorize \
	-flto \
	-ffast-math \
	-Wall \
	-Wno-missing-braces \
	-Iinclude \
	-Ideps/common.h \
	src/main.c \
	-o raster \
	`pkg-config --libs --cflags sdl2` \
	-lm \
	-DNO_OMP \
	-DNO_NORMAL_BUFFER \
	-DNO_SIMD \
	-g \
	&& \
	strip raster

# -fopenmp=libomp
