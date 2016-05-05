#!/bin/sh
# ----------------------------------------------------------------------------
# ReMooD Doom Source Port <http://remood.org/>
#   Copyright (C) 2005-2016 GhostlyDeath <ghostlydeath@remood.org>
#     For more credits, see AUTHORS.
# ----------------------------------------------------------------------------
# ReMooD is under the GNU General Public License v3 (or later), see COPYING.
# ----------------------------------------------------------------------------
# Creates a note for the current day.

# Force C locale because some programs such as sed and grep run like -turbo 10 
export LC_ALL=C

# Directory of this script
__exedir="$(dirname -- "$0")"

# Requires command
if [ "$#" -lt "1" ]
then
	echo "Usage: $0 (command) [...]"
	exit 1
fi

# Get command and shift down
__cmd=$1
shift 1

# Get the current date in string form.
__nowyear="$(date +%Y)"
__nowmont="$(date +%m)"
__nowdayy="$(date +%d)"

# The file to create
__mkdtime="$__nowyear\/$__nowmont\/$__nowdayy"
__file="$__exedir/../dev-notes/$__sublet/$__nowyear/$__nowmont/$__nowdayy.mkd"

# Create if missing
if [ ! -f "$__file" ]
then
	"$__exedir/create.sh" "$__file"
	sed "s/YYYYMMDD/$__mkdtime/g" < "$__exedir/base.mkd" > "$__file"
fi

# Open it
"$__cmd" $* "$__file"

