# Makefile

INSTALL_PATH?=/usr/local/bin

CC=clang

PROG=pngtoc

FLAGS=-o ${PROG} -Wall -Os -I../../include -I../../deps/common.h -lm

SRC=pngtoc.c

all: compile

prepare:

compile: prepare
	${CC} ${SRC} ${FLAGS}
	strip ${PROG}

install:
	chmod o+x ${PROG}
	cp ${PROG} ${INSTALL_PATH}

uninstall:
	rm ${INSTALL_PATH}/${PROG}
