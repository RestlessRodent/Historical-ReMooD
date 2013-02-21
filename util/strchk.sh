#!/bin/sh

# Clear found file
rm -f /tmp/$$

# Search all files with strings
grep 'DSTR_[a-zA-Z0-9_]*' < src/dstrings.h | sed 's/[^a-zA-Z0-9_]//g' | sed 's/DSTR_//g' | while read line
do
	# Progress
	echo -n "." 1>&2
	
	# Search each file
	for file in $(ls src/*.[ch] | sed 's/src\/dstrings\.[ch]//g') $(ls wad/lumps/*.lmp)
	do
		# Look in file for string
		if grep -Hn "$line" "$file"	2> /dev/null > /dev/null
		then
			echo "$line" >> /tmp/$$
			echo -n "#" 1>&2
		fi
	done
done

# Dump found list
sort < /tmp/$$ | uniq

