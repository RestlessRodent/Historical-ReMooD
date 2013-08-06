#!/bin/sh

LASTFILE=""
touch /tmp/$$

while read line
do
	SRCFILE="$(echo $line | cut -d ' ' -f 1)"
	THISFILE="$(echo $SRCFILE | cut -d ':' -f 1)"
	NEEDFUNC="$(echo $line | cut -d ' ' -f 2)"
	
	# Change of file?
	if [ "$THISFILE" != "$LASTFILE" ]
	then
		sort < /tmp/$$ | uniq
		
		LASTFILE=$THISFILE
		echo "*** $THISFILE"
		
		rm -f /tmp/$$
		touch /tmp/$$
	fi
	
	# Header it is inside
	GREPLINE=$(grep -rHn $NEEDFUNC *.h | head -n 1)
	USETHIS=$(echo $GREPLINE | cut -d ':' -f 1)
	
	echo '#include "'$USETHIS'"' >> /tmp/$$
done < "$1"

sort < /tmp/$$ | uniq

rm -f /tmp/$$

