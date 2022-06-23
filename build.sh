#!/bin/sh
set -xe

cc -Wall -Wextra -Wshadow -g -lm ../src/main.c -I../src -o imp