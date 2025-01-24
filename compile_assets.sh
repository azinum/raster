#!/bin/sh

FILE=include/assets.h
TOOLS_PATH=$1

echo "// assets.h" > ${FILE}
echo "#ifndef _ASSETS_H" >> ${FILE}
echo "#define _ASSETS_H" >> ${FILE}

for OBJ_PATH in data/mesh/*.obj; do
	NAME=${OBJ_PATH%.obj}
	NAME=${NAME##*/}
	${TOOLS_PATH}objtoc ${OBJ_PATH} ${NAME} >> ${FILE}
done

for PNG_PATH in data/texture/*.png; do
	NAME=${PNG_PATH%.png}
	NAME=${NAME##*/}
	${TOOLS_PATH}pngtoc ${PNG_PATH} ${NAME} >> ${FILE}
done

for TTF_PATH in data/font/*.ttf; do
	NAME=${TTF_PATH%.ttf}
	NAME=${NAME##*/}
	${TOOLS_PATH}font2c ${TTF_PATH} -n ${NAME} >> ${FILE}
done

${TOOLS_PATH}lutgen >> ${FILE}

echo "#endif // _ASSETS_H" >> ${FILE}
