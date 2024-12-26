# Makefile

CC=gcc

PROG=bintoc

FLAGS=-o ${PROG} -Wall -O3

SRC=bintoc.c

all: compile

prepare:

compile: prepare
	${CC} ${SRC} ${FLAGS}
	strip ${PROG}

install:
	chmod o+x bintoc
	cp bintoc /usr/local/bin

uninstall:
	rm /usr/local/bin/bintoc
