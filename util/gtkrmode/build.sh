#!/bin/sh

gcc -o gtkrmode -g3 -O0 `pkg-config gtk+-2.0 --cflags --libs` *.c

