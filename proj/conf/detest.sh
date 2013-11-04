#!/bin/sh
#         :oCCCCOCoc.
#     .cCO8OOOOOOOOO8Oo:
#   .oOO8OOOOOOOOOOOOOOOCc
#  cO8888:         .:oOOOOC.                                                TM
# :888888:   :CCCc   .oOOOOC.     ###      ###                    #########
# C888888:   .ooo:   .C########   #####  #####  ######    ######  ##########
# O888888:         .oO###    ###  #####  ##### ########  ######## ####    ###
# C888888:   :8O.   .C##########  ### #### ### ##    ##  ##    ## ####    ###
# :8@@@@8:   :888c   o###         ### #### ### ########  ######## ##########
#  :8@@@@C   C@@@@   oo########   ###  ##  ###  ######    ######  #########
#    cO@@@@@@@@@@@@@@@@@Oc0
#      :oO8@@@@@@@@@@Oo.
#         .oCOOOOOCc.                                      http://remood.org/
# -----------------------------------------------------------------------------
# Copyright (C) 2013-2013 GhostlyDeath <ghostlydeath@remood.org>
#                                      <ghostlydeath@gmail.com>
# -----------------------------------------------------------------------------
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 3
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# -----------------------------------------------------------------------------
# Tests the DJGPP Compiler in DOSEMU

# DosEMU is here
DOSEMUPATH="$1"

# ReMooD Root is here
REMOODDIR="$2"

# Cached?
if [ -f /tmp/remood-dosemu-cache ]
then
	echo "+++ DOSEMU Cached Check" 1>&2
	echo "ok"
	exit 0
fi

# Auto fail if there is no unix2dos
if ! which unix2dos 2> /dev/null > /dev/null
then
	exit 1
fi

# Remember current PWD
LASTPWD="$PWD"

# Setup temporary directory to test a native DJGPP on
cd /tmp
cp $REMOODDIR/util/hello.c $$.c
echo "gcc -o $$.exe $$.c -lalleg" > $$.bat
unix2dos $$.bat 2> /dev/null > /dev/null

# Run DOSEMU with just this batch file
# Run it in a background thread (so if it takes forever, do not bother)
# On my shitty Intel Atom 1.6GHz, this takes about 7 seconds to run
($DOSEMUPATH -I 'video {none}' -I 'sound_emu off' /tmp/$$.bat) > /dev/null 2> /dev/null &
BGTHREAD=$!

# Wait 1 minute for it to complete
for i in $(seq 1 60)
do
	# Process dead?
	if ! ps -p $BGTHREAD > /dev/null 2> /dev/null
	then
		break
	fi
	
	sleep 1
	echo -n "." 1>&2
done

# See if the process is alive, if it is kill -9 it...
if ps -p $BGTHREAD > /dev/null 2> /dev/null
then
	kill -9 $BGTHREAD > /dev/null 2> /dev/null
	
	# Fail regardless (EXE may be bluntly truncated)
	rm -f /tmp/$$.exe
fi

# Regardless, add a newline
echo "" 1>&2

# Remove batch and C file
rm -f $$.bat
rm -f $$.c

# go back to old directory
cd $LASTPWD

# See if it worked OK
if [ -f "/tmp/$$.exe" ]
then
	echo "ok"
	rm -f /tmp/$$.exe
	touch /tmp/remood-dosemu-cache
	exit 0
else
	exit 1
fi

