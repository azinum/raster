# Makefile

INSTALL_PATH?=/usr/local/bin

CC=clang

PROG=objtoc

FLAGS=-o ${PROG} -Wall -O3 -I../../include -I../../deps/common.h

SRC=objtoc.c

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
