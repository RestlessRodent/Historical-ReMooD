#!/bin/sh

# This looks for which strings are used, and which are not used anywhere
# Helpful for purging any unused strings that may exist.
# This second part list strings that do not exist at all in the found list

# Use strchk.sh before this

# Search all files with strings
grep 'DSTR_[a-zA-Z0-9_]*' < src/dstrings.h | sed 's/[^a-zA-Z0-9_]//g' | sed 's/DSTR_//g' | while read line
do
	# Only print line if it does not exist
	if ! grep "$line" "$1" 2> /dev/null > /dev/null
	then
		echo "$line"
	fi
done

