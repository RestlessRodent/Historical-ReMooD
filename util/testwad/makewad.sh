#!/bin/sh

gcc ../../util/rmdtex.c
./a.out wadinfo.txt ../../rtestwad.wad

## if we aren't already inside
#cd wad
#
## Clear old add
#rm -f ../bin/remood.wad
#
## Make new WAD
#deutex -rgb 0 255 255 -doom2 bootstrap -build wadinfo.txt ../bin/remood.wad

