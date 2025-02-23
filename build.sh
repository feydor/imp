#!/bin/sh
set -xe
SRC="src/main.c src/vector.c src/image.c src/system/bmp.c \
src/imp.c src/ui/toolmenu.c src/ui/actionmenu.c src/ui/colormenu.c src/system/palette.c \
src/canvas.c src/cursor.c"
CFLAGS="-Wall -Wextra -Wshadow -Wno-unused-function -g"

mkdir -p build

if [[ "$OSTYPE" == "linux-gnu"* ]]; then
    cc $CFLAGS $SRC -I src -o imp `sdl2-config --cflags --libs` -lSDL2_image -lm
elif [[ "$OSTYPE" == "msys" ]]; then
    # include and lib folders from SDL2-devel/i686-w64-mingw32
    cc $CFLAGS $SRC -I src -I include -L lib -o build/imp -lSDL2_image -lmingw32 -lSDL2main -lSDL2 -lm
else
	# assumming cygwin
    # include and lib folders from SDL2-devel/i686-w64-mingw32
    cc $CFLAGS $SRC -I src -I include -L lib -o build/imp -lSDL2_image -lmingw32 -lSDL2main -lSDL2
fi

