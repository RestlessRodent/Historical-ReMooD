#!/bin/sh
# -----------------------------------------------------------------------------
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
# Copyright (C) 2009-2014 GhostlyDeath <ghostlydeath@remood.org>
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
# DESCRIPTION: Exports fossil image

if [ -n "$1" ]
then
	TARGET="$1"
else
	TARGET="/tmp/remood.fsl"
fi

if [ -f /tmp/remood-fossil-pid ]
then
	kill $(cat /tmp/remood-fossil-pid)
	rm -f /tmp/remood-fossil-pid
fi

export LC_ALL=C

PORT="$(expr $(expr $$ % 32765) + 32765)"

fossil serve -P $PORT &
FOSSILPID=$!
echo $FOSSILPID > /tmp/remood-fossil-pid
fossil clone -A remood "http://127.0.0.1:$PORT" \
	"/tmp/$$.fsl"
kill -- $FOSSILPID
wait
rm -f -- /tmp/remood-fossil-pid

# If target exists, Do a compressed rebuild
if [ -f "/tmp/$$.fsl" ]
then
	RPASS="$( (sha1sum "/tmp/$$.fsl" || md5sum "/tmp/$$.fsl") | tr '\t' ' ' | \
		cut -d ' ' -f 1)"
	echo "Repository password is: '$RPASS'" 1>&2
	fossil user password remood "$RPASS" -R "/tmp/$$.fsl"
	fossil user default remood -R "/tmp/$$.fsl"
	
	CAPNOBO="ghjorz"
	fossil user capabilities nobody $CAPNOBO -R "/tmp/$$.fsl"
	
	CAPANON="$CAPNOBO"
	fossil user capabilities anonymous $CAPANON -R "/tmp/$$.fsl"
	
	CAPREAD="kptwhmnczgjor"
	fossil user capabilities reader $CAPREAD -R "/tmp/$$.fsl"

	CAPDEVE="deihmnczgjor"
	fossil user capabilities developer $CAPDEVE -R "/tmp/$$.fsl"
	
	fossil remote-url -R "/tmp/$$.fsl" off
	fossil remote-url -R "/tmp/$$.fsl" \
		http://remood.org:8080/remood/
	fossil settings -R "/tmp/$$.fsl" autosync 0
	fossil settings -R "/tmp/$$.fsl" case-sensitive 1
	
	fossil config push shun "/tmp/$$.fsl"
	
	fossil rebuild --force --vacuum --pagesize 1024 --compress \
		--stats "/tmp/$$.fsl"
	
	fossil zip tip "/tmp/$$.zip" --name "remood" -R "/tmp/$$.fsl"
	
	cat "/tmp/$$.fsl" "/tmp/$$.zip" > "$TARGET"
	ls -sh "/tmp/$$."??? "$TARGET" | sort -h
	
	rm -f "/tmp/$$.fsl" "/tmp/$$.zip" "/tmp/$$.tar" "/tmp/$$.tgz" \
		"/tmp/$$.txz" "/tmp/$$.tb2"
else
	exit 1
fi

echo "Done." 1>&2
exit 0

