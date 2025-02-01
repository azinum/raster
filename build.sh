#!/bin/bash

set -xe

OPT=-O3
clang \
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
	-fopenmp=libomp \
	-Iinclude \
	-Ideps/common.h \
	src/main.c \
	-o raster \
	`pkg-config --libs --cflags sdl2` \
	-lm \
	-DNO_NORMAL_BUFFER \
	-DNO_SIMD \
	&& \
	strip raster
