# Makefile

CC=clang

PROG=lutgen

FLAGS=-o ${PROG} -Wall -O3 -I../../deps/common.h -I../../include -lm

SRC=lutgen.c

all: compile

prepare:

compile: prepare
	${CC} ${SRC} ${FLAGS}
	strip ${PROG}

install:
	chmod o+x ${PROG}
	cp ${PROG} /usr/local/bin

uninstall:
	rm /usr/local/bin/${PROG}
