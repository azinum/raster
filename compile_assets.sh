#!/bin/sh

FILE=include/assets.h

echo "// assets.h" > ${FILE}
echo "#ifndef _ASSETS_H" >> ${FILE}
echo "#define _ASSETS_H" >> ${FILE}

for OBJ_PATH in data/mesh/*.obj; do
	NAME=${OBJ_PATH%.obj}
	NAME=${NAME##*/}
	objtoc ${OBJ_PATH} ${NAME} >> ${FILE}
done

for PNG_PATH in data/texture/*.png; do
	NAME=${PNG_PATH%.png}
	NAME=${NAME##*/}
	pngtoc ${PNG_PATH} ${NAME} >> ${FILE}
done

for TTF_PATH in data/font/*.ttf; do
	NAME=${TTF_PATH%.ttf}
	NAME=${NAME##*/}
	font2c ${TTF_PATH} -n ${NAME} >> ${FILE}
done

lutgen >> ${FILE}

echo "#endif // _ASSETS_H" >> ${FILE}
