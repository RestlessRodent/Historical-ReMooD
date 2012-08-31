#!/bin/sh

# Convert Objects
if ! [ -f her_mos.tsv ]
then
	(sed 's/[\ \t\r]//g' < hereinfo.c | sed 's/{\/\///g' | sed 's/}/%%%/g' | sed 's/,//g' | sed 's/\*FRACUNIT/.0/g' | sed 's/\/\/.*$//g' | while read line
	do
		if echo $line | grep '%%%' > /dev/null
		then
			echo ""
		else
			echo -n "$line\t"
		fi
	done) > her_mos.tsv
fi

# Convert States
if ! [ -f her_stt.tsv ]
then
	(sed 's/\([^\/]*\)\/\/[ \t]\{0,\}\([A-Za-z0-9_]*\)$/\2\1/g' < herstat.c | sed 's/[{}]//g' | sed 's/[ \t]/,/g' | sed 's/,\{1,\}/\t/g' | while read line
	do
		echo "$line"
	done) > her_stt.tsv
fi

