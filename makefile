# ########   ###### #####   #####  ######   ######  ######
# ##     ##  ##     ##  ## ##  ## ##    ## ##    ## ##   ##
# ##     ##  ##     ##   ###   ## ##    ## ##    ## ##    ##
# ########   ####   ##    #    ## ##    ## ##    ## ##    ##
# ##    ##   ##     ##         ## ##    ## ##    ## ##    ##
# ##     ##  ##     ##         ## ##    ## ##    ## ##   ##
# ##      ## ###### ##         ##  ######   ######  ######
#                      http://remood.sourceforge.net/
# -----------------------------------------------------------------------------
# Project Leader:    GhostlyDeath           (ghostlydeath@gmail.com)
# Project Co-Leader: RedZTag                (jostol27@gmail.com)
# Members:           Demyx                  (demyx@endgameftw.com)
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
# BCC/CL Makefile (Borland and Microsoft)

################
### BINARIES ###
################

!ifdef BORLAND
	CC = $(BINPREFIX)bcc$(BINSUFFIX)
	RCC = $(BINPREFIX)brcc32$(BINSUFFIX)
!else
	CC = cl
	RCC = rc
	
	MICROSOFTDIRECTIVES = 1
!endif

###################
### DIRECTORIES ###
###################

INPUT_SRC  = src
INPUT_ISRC = isrc
INPUT_RC   = rc
INPUT_WAD  = wad
OUTPUT_EXE = bin
OUTPUT_WAD = bin

!ifdef ALTERNATEOBJDIR
	OUTPUT_OBJ = $(ALTERNATEOBJDIR)
!else
	OUTPUT_OBJ = objs
!endif

####################################
### SYSTEM SPECIFIC DECLARATIONS ###
####################################

# Always Windows
OSDETECT_WINDOWS = 1
OSHOST_WINDOWS = 1

#################
### SUBSYSTEM ###
#################

# Always SDL
SUBSYSTEM_SDL = 1

!ifdef MICROSOFTDIRECTIVES
	!ifdef SDL_INCLUDE
		CC_SUBSYSTEMCFLAGS = /I$(SDL_INCLUDE)
	!else
		CC_SUBSYSTEMCFLAGS = /I. /ISDL
	!endif

	!ifdef SDL_LIB
		CC_SUBSYSTEMLFLAGS = /L$(SDL_LIB)
	!else
		CC_SUBSYSTEMLFLAGS = /L. /LSDL
	!endif
!else
	!ifdef SDL_INCLUDE
		CC_SUBSYSTEMCFLAGS = -I$(SDL_INCLUDE)
	!else
		CC_SUBSYSTEMCFLAGS = -I. -ISDL
	!endif

	!ifdef SDL_LIB
		CC_SUBSYSTEMLFLAGS = -L$(SDL_LIB)
	!else
		CC_SUBSYSTEMLFLAGS = -L. -LSDL
	!endif
!endif

