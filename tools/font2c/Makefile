# Makefile

INSTALL_PATH?=/usr/local/bin

CC=clang

PROG=font2c

FLAGS=-o ${PROG} -Wall -O3 `pkg-config freetype2 --libs --cflags`

SRC=font2c.c

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
