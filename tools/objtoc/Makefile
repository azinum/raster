# Makefile

CC=clang

PROG=objtoc

FLAGS=-o ${PROG} -Wall -O3 -I../../deps/common.h -I../../include

SRC=objtoc.c

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

