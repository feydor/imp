#!/bin/sh
set -xe
SRC="../src/main.c ../src/vector.c ../src/image.c ../src/bmp.c"
CFLAGS="-Wall -Wextra -Wshadow -g"

cc $CFLAGS $SRC -I../src -o imp `sdl2-config --cflags --libs` -lm