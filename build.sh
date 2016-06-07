#!/bin/sh
# ---------------------------------------------------------------------------
# ReMooD Doom Source Port <http://remood.org/>
#   Copyright (C) 2005-2016 GhostlyDeath <ghostlydeath@remood.org>
#     For more credits, see readme.mkd.
# ---------------------------------------------------------------------------
# ReMooD is under the GNU General Public License v3+, see license.mkd.
# ---------------------------------------------------------------------------

# Location of this script
__exenam="$0"
__exedir="$(dirname "$0")"

# Prints usage
__print_usage()
{
	# Usage
	echo "Usage: $__exenam [-i java_include] [-l java_lib] \
[-t target]" 1>&2
}

# Read options
__java_inc=""
__java_lib=""

# Parse input operations
while getopts "i:l:" __arg
do
	# Depends
	case "$__arg" in
		i)
			__java_inc="$OPTARG"
			;;
			
		l)
			__java_lib="$OPTARG"
			;;
			
		?)
			__print_usage
			exit 1
			;;
	esac
done

# These must be set
if [ -z "$__java_inc" ] || [ -z "$__java_lib" ]
then
	__print_usage
	exit 1
fi

echo "$__java_inc $__java_lib"

exit 2
