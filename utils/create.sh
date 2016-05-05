#!/bin/sh
# ----------------------------------------------------------------------------
# ReMooD Doom Source Port <http://remood.org/>
#   Copyright (C) 2005-2015 GhostlyDeath <ghostlydeath@remood.org>
#     For more credits, see AUTHORS.
# ----------------------------------------------------------------------------
# ReMooD is under the GNU General Public License v3 (or later), see COPYING.
# ----------------------------------------------------------------------------

# Force C locale because some programs such as sed and grep run like -turbo 10 
export LC_ALL=C

# Directory of this script
__exedir="$(dirname -- "$0")"

# Base initialization
__cmd=""
__makex="0"
__noadd="0"
__notemplate="0"

# Usage
print_usage()
{
	echo "Usage: $0 [-b] [-c command] [-n] [-x] [files ...]"
	echo ""
	echo " -b Use blank file."
	echo " -c Run files with a command."
	echo " -n Do not 'fossil add foo'."
	echo " -x chmod +x each created file."
}

# Handle arguments
while getopts "bnxc:" FOO
do
	case $FOO in
		b)
			__notemplate="1"
			;;
			
		c)
			__cmd="$OPTARG"
			;;
		
		n)
			__noadd="1"
			;;
		
		x)
			__makex="1"
			;;
		
		*)
			print_usage
			exit 1
			;;
	esac 
done

# Down they go
shift $(($OPTIND - 1))

# All files
__files="$*"

# Not many systems have a readlink, so this is required
__absolutepath()
{
	# path is absolute, just needs . and .. removed
	if echo "$1" | grep '^\/' > /dev/null
	then
		__yuck="$1"
	
	# Get relative path from PWD
	else
		__yuck="$(pwd)/$1"
	fi

	# Split path by slashes and handle each line
	# Need file due to argument stuff.
	rm -f /tmp/remood$$.ap
	touch /tmp/remood$$.ap
	echo "$__yuck" | sed 's/\//\n/g' | while read __seg
	do
		# If this segment is ".", ignore it
		# Also ignore blank segments too
		if [ "$__seg" = "." ] || [ -z "$__seg" ]
		then
			continue
	
		# If this is .., remove the topmost path element
		elif [ "$__seg" = ".." ]
		then
			sed 's/\/[^/]*$//g' < /tmp/remood$$.ap > /tmp/remood$$.ap2
			mv /tmp/remood$$.ap2 /tmp/remood$$.ap
	
		# Otherwise append to path
		else
			echo -n "/$__seg" >> /tmp/remood$$.ap
		fi
	done

	# Blank result ends in /
	__result="$(cat /tmp/remood$$.ap)"
	rm -f /tmp/remood$$.ap
	if [ -z "$__result" ]
	then
		echo "/"
	else
		echo "$__result"
	fi
}

# Find the name of the package based on the directory it is in
__findpkname()
{
	# Get directory file is in
	__indir="$(dirname "$(__absolutepath "$1")")"
	
	# Directory loop
	__pkout="org.remood"
	__chop=""
	while [ "$__indir" != "/" ]
	do
		# Is the root of the source tree?
		if [ -f "$__indir/../../build.xml" ] || \
			[ -f "$__indir/../build.xml" ] || \
			[ -f "$__indir/build.xml" ]
		then
			__pkout="$__chop"
			break
		fi
		
		# Get base name and chop down
		if [ -z "$__chop" ]
		then
			__chop="$(basename "$__indir")"
		else
			__chop="$(basename "$__indir").$__chop"
		fi
		__indir="$(dirname "$__indir")"
	done
	
	# Print out the package name
	echo "$__pkout"
}

# Go through all arguments
while [ "$#" -gt "0" ]
do
	# Create upper directoroies
	mkdir -p "$(dirname -- "$1")"	
	
	# Create file
	if [ ! -f "$1" ]
	then
		# From a template?
		if [ "$__notemplate" -eq "0" ]
		then
			# Get the absolute path of the current file
			__tabsf="$(__absolutepath "$1")"
			
			# Get location and name of file
			__tfile="$(basename -- "$__tabsf")"
			__tidir="$(dirname -- "$__tabsf")"
			
			# Get extension of file
			__tbase="$(echo -n "$__tfile" | sed 's/\(.*\)\..*$/\1/')"
			__tfext="$(echo -n "$__tfile" | sed 's/.*\.\(.*\)$/\1/')"
			
			# Name of the package
			__tpack="$(__findpkname "$__tabsf")"
			
			# Name of class
			__tclas="$__tbase"
			
			# Java source code
			(if [ "$__tfext" = "java" ]
			then
				# Package file
				if [ "$__tbase" = "package-info" ]
				then
					cat "$__exedir/base-package-info.java" 
				else
					cat "$__exedir/base.java"
				fi
			
			# C source
			elif [ "$__tfext" = "c" ]
			then
				cat "$__exedir/base.c"
			
			# C header
			elif [ "$__tfext" = "h" ]
			then
				cat "$__exedir/base.h"
			
			# Resource bundle
			elif [ "$__tfext" = "properties" ]
			then
				cat "$__exedir/base.properties"
			
			# Unknown, blank file
			else
				echo
			fi) | sed 's/REMOOD_CLASS/'"$__tclas"'/g' \
				| sed 's/REMOOD_PACKAGE/'"$__tpack"'/g' > "$1"
		
		# Not wanting a template
		else
			# Touch base file so it exists
			touch "$1"
		fi
	fi
	
	# Make it executable
	if [ "$__makex" -ne "0" ]
	then
		chmod +x "$1"
	fi
	
	# Add to repo
	if [ "$__noadd" -eq "0" ]
	then
		fossil add "$1"
	fi
	
	# Done with file
	shift 1
done

# Pass all input files to the command
if [ -n "$__cmd" ] && [ -n "$__files" ]
then
	"$__cmd" $__files
fi

