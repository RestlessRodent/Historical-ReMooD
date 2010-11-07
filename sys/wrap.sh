#!/bin/sh

# ########   ###### #####   #####  ######   ######  ######
# ##     ##  ##     ##  ## ##  ## ##    ## ##    ## ##   ##
# ##     ##  ##     ##   ###   ## ##    ## ##    ## ##    ##
# ########   ####   ##    #    ## ##    ## ##    ## ##    ##
# ##    ##   ##     ##         ## ##    ## ##    ## ##    ##
# ##     ##  ##     ##         ## ##    ## ##    ## ##   ##
# ##      ## ###### ##         ##  ######   ######  ######
#                      http://remood.org/
# -----------------------------------------------------------------------------
# Project Leader:    GhostlyDeath           (ghostlydeath@remood.org)
# Project Co-Leader: RedZTag                (jostol@remood.org)
# -----------------------------------------------------------------------------
# Copyright (C) 2009 The ReMooD Team.
# -----------------------------------------------------------------------------
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# -----------------------------------------------------------------------------
# Wrapper for ReMooD

if [ "`echo $DOOMWADPATH | grep "/usr/share/games/doom"`" == "" ]
then
	if [ "`echo $DOOMWADPATH`" == "" ]
	then
		export DOOMWADPATH="/usr/share/games/doom"
	else
		export DOOMWADPATH="$DOOMWADPATH:/usr/share/games/doom"
	fi
fi
	
if [ "`echo $DOOMWADPATH | grep "/usr/local/share/games/doom"`" == "" ]
then
	
	if [ "`echo $DOOMWADPATH`" == "" ]
	then
		export DOOMWADPATH="/usr/local/share/games/doom"
	else
		export DOOMWADPATH="$DOOMWADPATH:/usr/local/share/games/doom"
	fi
fi

remood.real $@

