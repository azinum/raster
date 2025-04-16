# Makefile

INSTALL_PATH?=/usr/local/bin

CC=gcc

PROG=bintoc

FLAGS=-o ${PROG} -Wall -O3 -Wall

SRC=bintoc.c

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
