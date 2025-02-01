# Makefile

COMPILER?=clang
OPT?=3
CC=${COMPILER}
FLAGS?=-Wall -O${OPT} -I../deps/common.h -I../include -iquote ../src -flto -ffast-math -Wall -Wno-missing-braces -fopenmp=libomp
LIBS?=-lm
TARGETS=${subst .c,, ${shell find . -type f -name '*.c'}}

all: ${TARGETS}

%: %.c
	${CC} $< -o $@ ${FLAGS} ${LIBS}
	${CC} $< -o $@-no-simd ${FLAGS} ${LIBS} -DNO_SIMD

clean:
	@for prog in ${TARGETS}; do \
		rm $${prog}; \
		rm $${prog}-no-simd; \
	done

run:
	@for prog in ${TARGETS}; do \
		echo $$prog; \
		$${prog}; \
		echo $${prog}-no-simd; \
		$${prog}-no-simd; \
	done

.PHONY: clean
