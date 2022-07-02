#!/bin/sh
set -xe
SRC="../src/main.c ../src/vector.c ../src/image.c ../src/system/bmp.c \
../src/imp.c ../src/layer.c ../src/ui/button_bar.c ../src/system/palette.c"
CFLAGS="-Wall -Wextra -Wshadow -g"

cc $CFLAGS $SRC -I../src -o imp `sdl2-config --cflags --libs` -lm