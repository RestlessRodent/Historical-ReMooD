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
# Members:           TetrisMaster512        (tetrismaster512@hotmail.com)
# -----------------------------------------------------------------------------
# Copyright (C) 2010 The ReMooD Team.
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
# .app wrapper

mkdir ./ReMooD.app
mkdir ./ReMooD.app/Contents
mkdir ./ReMooD.app/Contents/Resources
mkdir ./ReMooD.app/Contents/MacOS
mkdir ./ReMooD.app/Contents/Frameworks
cp ./remood ./ReMooD.app/Contents/MacOS/remood
cp ./Info.plist ./ReMooD.app/Contents/Info.plist
cp ./remood.icns ./ReMooD.app/Contents/Resources/remood.icns
cp -R ./SDL.framework ./ReMooD.app/Contents/Frameworks/SDL.framework
