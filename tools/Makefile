# Makefile

INSTALL_PATH?=~/.local/bin

all:
	make -C bintoc
	make -C font2c
	make -C lutgen
	make -C objtoc
	make -C pngtoc

install:
	make -C bintoc install INSTALL_PATH=${INSTALL_PATH}
	make -C font2c install INSTALL_PATH=${INSTALL_PATH}
	make -C lutgen install INSTALL_PATH=${INSTALL_PATH}
	make -C objtoc install INSTALL_PATH=${INSTALL_PATH}
	make -C pngtoc install INSTALL_PATH=${INSTALL_PATH}

uninstall:
	make -C bintoc uninstall INSTALL_PATH=${INSTALL_PATH}
	make -C font2c uninstall INSTALL_PATH=${INSTALL_PATH}
	make -C lutgen uninstall INSTALL_PATH=${INSTALL_PATH}
	make -C objtoc uninstall INSTALL_PATH=${INSTALL_PATH}
	make -C pngtoc uninstall INSTALL_PATH=${INSTALL_PATH}
