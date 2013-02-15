#!/bin/bash

for i in $(seq 33 126)
do
	BASE="UFNU$(printf %04X $i)"
	FILE="ufnu$(printf %04x $i).ppm"
	pbmtext -font ter-u12n.bdf "$(printf "\\x$(printf '%02x' $i)")" > /tmp/$FILE 
	
	# Crop
	pnmcrop -left -right /tmp/$FILE > /tmp/$$
	mv /tmp/$$ /tmp/$FILE
	
	# Cut
	pnmcut -top 6 -bottom -7 /tmp/$FILE > /tmp/$$
	mv /tmp/$$ /tmp/$FILE
	
	# Get original base size of PBM
	PX="$(identify -format "%w" /tmp/$FILE)"
	PY="$(identify -format "%h" /tmp/$FILE)"
	
	# Crop the top
	pnmcrop -top /tmp/$FILE > /tmp/$$
	mv /tmp/$$ /tmp/$FILE
	
	# Get new size
	CX="$(identify -format "%w" /tmp/$FILE)"
	CY="$(identify -format "%h" /tmp/$FILE)"
	
	# Crop the bottom
	pnmcrop -bottom /tmp/$FILE > /tmp/$$
	mv /tmp/$$ /tmp/$FILE
	
	# Pad width to 6 (Terminus Size)
	pnmpad -white -width 6 /tmp/$FILE > /tmp/$$
	mv /tmp/$$ /tmp/$FILE
	
	# Add 1 extra pixel
	pnmpad -white -halign 0.0 -right 1 /tmp/$FILE > /tmp/$$
	mv /tmp/$$ /tmp/$FILE
	
	# Change Colors
	convert -fill '#00FF00' -opaque '#000000' /tmp/$FILE /tmp/$FILE 
	convert -fill '#00FFFF' -opaque '#FFFFFF' /tmp/$FILE /tmp/$FILE
	
	# Print said size, for wadinfo.txt
	echo "$BASE 0 $(expr $CY - 10)"
done

