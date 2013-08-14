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
# Downloads Allegro Windows 32-bit

# Prefix that the Windows toolchain uses
WINPREFIX="$1"

# ReMooD is passed via this directory
REMOODDIR="$2"

# Go to the ReMooD Dir
cd $REMOODDIR

# Download and extract the tar.gz, if needed
if [ ! -d win32all ]
then
	# See if the tgz exists
	if [ ! -f win32all.tgz ]
	then
		$REMOODDIR/proj/conf/dl.sh http://remood.org/downloads/win32all.tar.gz win32all.tgz
	fi
	
	# See if it was actually created
	if [ ! -f win32all.tgz ]
	then
		exit 1
	fi
	
	# Needs extracting
	tar -xvf win32all.tgz 1>&2
fi

# Seems to be ok
echo "ok"
exit 0

