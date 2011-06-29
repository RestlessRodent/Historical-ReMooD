#!/bin/sh -e

# Compile genunicd.c
gcc -O0 -g3 genunicd.c

# Download case change information
wget -c http://www.unicode.org/Public/UNIDATA/CaseFolding.txt -O CaseFolding.txt

# Run converter on it
./a.out cf CaseFolding.txt

