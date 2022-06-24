#!/bin/sh
set -xe

cc -Wall -Wextra -Wshadow -g -lm ../src/main.c ../src/vector.c ../src/image.c -I../src -o imp