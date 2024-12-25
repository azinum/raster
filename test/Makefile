# Makefile
# wasmtime --invoke <function> <wasm> arg1 arg2

CC=clang
OPT=-O0
FLAGS=--target=wasm32 -flto -ffast-math -nostdlib -Wl,--no-entry -Wl,--export-all -Wl,--allow-undefined ${OPT} -I../deps/common.h -I../include -DNO_STDLIB -DNO_STDIO -DTARGET_WASM -DNO_MATH

SRC=${subst .c,, ${wildcard *.c}}

all: ${SRC}

${SRC}:
	${CC} ${FLAGS} $@.c -o $@.wasm ${OPT} 
	wasm2wat $@.wasm -o $@.wat

clean:
	rm -f *.o *.wasm
