#!/bin/sh
set -xe
SRC="../src/main.c ../src/vector.c ../src/image.c ../src/system/bmp.c \
../src/imp.c ../src/layer.c ../src/ui/buttonpanel.c ../src/system/palette.c \
../src/ui/layermenu.c ../src/canvas.c"
CFLAGS="-Wall -Wextra -Wshadow -Wno-unused-function -g"

cc $CFLAGS $SRC -I../src -o imp `sdl2-config --cflags --libs` -lSDL2_image -lm