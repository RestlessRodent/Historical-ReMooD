#!/bin/sh

rm -f "$1.dstrh"
rm -f "$1.dstrc"
rm -f "$1.menu"

INBRACE="0"
cat "$1" | while read line
do
	echo "$line" | sed 's/[^"]*"\([a-zA-Z0-9_]*\).*/Item "\1"\n{\n\tText "MENUGENERAL_\1"\n\t\n\tOmniFunc "Variable";\n\tOmniVal "\$\1";\n}\n/g'
done | while read secline
do
	if echo "$secline" | grep "}" > /dev/null 2> /dev/null
	then
		INBRACE="0"
	fi
	
	if echo "$secline" | grep "MENUGENERAL" > /dev/null 2> /dev/null
	then
		VARNAME="$(echo "$secline" | sed 's/[^"]*"\([a-zA-Z0-9_]*\)"[^"]*/\1/')"
		UPPER="$(echo "$VARNAME" | sed 'y/qwertyuiopasdfghjklzxcvbnm/QWERTYUIOPASDFGHJKLZXCVBNM/')"
		FINAL="$(echo "$UPPER" | sed 's/_//g' | sed 's/MENUGENERAL/MENUGENERAL_/')"
		
		echo "DSTR_$FINAL," >> "$1.dstrh"
		printf "{%37s, \"TEXTHERE\"},\n" "\"$FINAL\"" | sed 's/    /\t/g' >> "$1.dstrc"
		
		if [ "$INBRACE" = "0" ]
		then
			echo "Text \"$FINAL\";" >> "$1.menu"
		else
			echo "\tText \"$FINAL\";" >> "$1.menu"
		fi
	else
		if [ "$INBRACE" = "0" ]
		then
			echo "$secline" >> "$1.menu"
		else
			echo "\t$secline" >> "$1.menu"
		fi
	fi
	
	if echo "$secline" | grep "{" > /dev/null 2> /dev/null
	then
		INBRACE="1"
	fi
done
