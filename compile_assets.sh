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

# for TTF_PATH in assets/font/*.ttf; do
# 	NAME=${TTF_PATH%.ttf}
# 	NAME=${NAME##*/}
# 	./font2c ${TTF_PATH} -n ${NAME} >> ${FILE}
# done

echo "#endif // _ASSETS_H" >> ${FILE}