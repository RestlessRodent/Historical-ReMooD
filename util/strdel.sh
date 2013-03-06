#!/bin/sh

# Deletes strings located in file

cp src/dstrings.c /tmp/$$.c
cp src/dstrings.h /tmp/$$.h

cat "$1" | while read line
do
	# Delete in header
	sed "/$line/d" < /tmp/$$.h > /tmp/$$
	mv /tmp/$$ /tmp/$$.h
	
	# Delete in source
	sed "/$line/d" < /tmp/$$.c > /tmp/$$
	mv /tmp/$$ /tmp/$$.c
done

mv /tmp/$$.h src/dstrings.h
mv /tmp/$$.c src/dstrings.c

