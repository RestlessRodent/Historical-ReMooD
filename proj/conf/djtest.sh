#!/bin/sh

# DJGPP's path is passed via the first argument
GCC="$1"

# ReMooD is passed via this directory
REMOODDIR="$2"

# Now determine whether or not it actually works
if ! $GCC -o /tmp/$$.exe $REMOODDIR/util/hello.c -lalleg > /dev/null 2> /dev/null
then
	# Failed miserably
	exit 1
fi

# It worked, so delete the output
rm -f /tmp/$$.exe

# And just say OK!
echo ok
exit 0

