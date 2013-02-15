#!/bin/sh

linemode=0
LINEA=""
LINEB=""
LINEC=""
snipped="0"

rm -f dsc
rm -f dsh
rm -f pdc

cat ../src/p_demcmp.c | while read -r line
do
		# First line mode
		if [ "$linemode" = "0" ]
		then
			# Line starts with {PEXGST_
			if echo "$line" | grep '[ \t]*{[ \t]*PEXGST_' 2> /dev/null > /dev/null
			then
				linemode="1"
				LINEA="$line"
				snipped="0"
			else
				LINEA=""
				LINEB=""
				LINEC=""
				
				if [ "snipped" = "0" ]
				then
					echo "... SNIP ..."
					snipped="1"
				fi
			fi
		
		# Second line mode
		elif [ "$linemode" = "1" ]
		then
			LINEB="$line"
			linemode="2"
			snipped="0"
		
		# Third line mode
		elif [ "$linemode" = "2" ]
		then
			LINEC="$line"
			
			#	{PEXGST_INTEGER, PGS_NOTHINGHERE, "nothinghere", "Nothing",
			#		"Nothing is here", PEXGSGM_ANY, PEXGSDR_NOCHECK, 0, {0, 0}, 0,
			#		PEXGSMC_NONE, PEXGSDA_YESNO, c_PEXGSPVBoolean, NULL},
			
			VARNAME=$(echo "$LINEA" | sed 's/.*\(PGS_[A-Z_]*\).*/\1/')
			MENUTEXT="$(echo "$LINEA" | sed 's/.*"[^"]*".*\("[^"]*"\).*/\1/')"
			DESCTEXT="$(echo "$LINEB" | sed 's/.*\("[^"]*"\).*/\1/')"
			#'
			
			# dstrings.c
			printf '\t{%34s, %s},\n' "\"M_$VARNAME\"" "$MENUTEXT" >> dsc
			printf '\t{%34s, %s},\n' "\"D_$VARNAME\"" "$DESCTEXT" >> dsc
			
			# dstrings.h
			printf '\tDSTR_%s,\n' M_$VARNAME >> dsh
			printf '\tDSTR_%s,\n' D_$VARNAME >> dsh
			
			# p_demcmp.c
			echo "\t$LINEA" | sed 's/\(.*"[^"]*".*\)"[^"]*"\(.*\)/\1xxx\2/' | sed "s/xxx/DSTR_M_$VARNAME/" >> pdc
			echo "\t\t$LINEB" | sed 's/\(.*\)"[^"]*"\(.*\)/\1xxx\2/' | sed "s/xxx/DSTR_D_$VARNAME/" >> pdc
			echo "\t\t$LINEC" >> pdc
			echo "" >> pdc
			
			# Reset
			linemode="0"
			snipped="0"
		fi
		
done

