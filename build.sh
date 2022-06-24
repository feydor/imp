#!/bin/sh
set -xe

cc -Wall -Wextra -Wshadow -g -lm ../src/main.c ../src/vector.c -I../src -o imp