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
# Downloads a file from the internet

# Use wget, since I know that the most
if which wget 2> /dev/null > /dev/null
then
	wget -c --progress=dot "$1" -O "$2" 1>&2 || rm -f "$2"

# Curl
elif which curl 2> /dev/null > /dev/null
then
	curl -C -f -o "$2" "$1" 1>&2 || rm -f "$2"

# No downloader available, very sad =(
else
	exit 1
fi
	
# If file exists, download worked
if [ -f "$2" ]
then
	exit 0
fi

# Failed
exit 1


