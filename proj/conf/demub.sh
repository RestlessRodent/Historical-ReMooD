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
# Builds script wrapper for building on DOSEMU (Easier with script than make)

# DosEMU is here
DOSEMUPATH="$1"

# ReMooD Root is here
REMOODDIR="$2"

# Go to the ReMooD Dir, builds are initiated from there
cd "$REMOODDIR"

# Make WAD file (it will take a REALLY long time to build on DOSEMU)
make wad

# Create batch file which will clean and build
echo "make clean
make
" > $$.bat

# Convert to DOS format
unix2dos $$.bat

# Enter DOSEMU and execute
$DOSEMUPATH -I 'video {none}' -I 'sound_emu off' $$.bat

# Cleanup after oneself
rm -f $$.bat

# If remood.exe exists, the build worked
if [ -f "bin/remood.exe" ]
then
	exit 0

# Otherwise the build failed for some reason
else
	exit 1
fi

