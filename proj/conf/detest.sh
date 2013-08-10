#!/bin/sh

# DosEMU is here
DOSEMUPATH="$1"

# ReMooD Root is here
REMOODDIR="$2"

# Auto fail if there is no unix2dos
if ! which unix2dos 2> /dev/null > /dev/null
then
	exit 1
fi

# Remember current PWD
LASTPWD="$PWD"

# Setup temporary directory to test a native DJGPP on
cd /tmp
cp $REMOODDIR/util/hello.c $$.c
echo "gcc -o $$.exe $$.c -lalleg" > $$.bat
unix2dos $$.bat 2> /dev/null > /dev/null

# Run DOSEMU with just this batch file
$DOSEMUPATH /tmp/$$.bat 2> /dev/null > /dev/null

# Remove batch file
rm -f $$.bat

# go back to old directory
cd $LASTPWD

# See if it worked OK
if [ -f "/tmp/$$.exe" ]
then
	echo "ok"
	rm -f /tmp/$$.exe
	exit 0
else
	exit 1
fi

