#!/bin/sh
yasm -g stabs -f elf64 -o bin/sad.o src/sad.s
gcc -o bin/sad bin/sad.o
