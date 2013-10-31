#!/usr/bin/make -f
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
# Allegro 4.x


# Use allegro-config if ALLEGRO_LIB and ALLEGRO_INCLUDE are not set
ifeq (,$(strip $(ALLEGRO_LIB))$(strip $(ALLEGRO_INCLUDE)))
	ifneq (,$(findstring 4.2,$(strip $(shell $(call ___RUNCOMMAND,$(CONFIGPREFIX)allegro-config --version)))))
		export ___ALLEGROCFLAGS  := $(shell $(call ___RUNCOMMAND,$(CONFIGPREFIX)allegro-config --cflags))
		export ___ALLEGROLDFLAGS := $(shell $(call ___RUNCOMMAND,$(CONFIGPREFIX)allegro-config --libs))
	endif
endif

# Fall back to environment variables
ifeq (,$(strip $(___ALLEGROCFLAGS)))
	ifneq (,$(strip $(ALLEGRO_INCLUDE)))
		export ___ALLEGROCFLAGS  := -I$(ALLEGRO_INCLUDE)
	else
		export ___ALLEGROCFLAGS  := -Iinclude -Iallegro
	endif
endif

ifeq (,$(strip $(___ALLEGROLDFLAGS)))
	ifneq (,$(strip $(ALLEGRO_LIB)))
		export ___ALLEGROLDFLAGS := -L$(ALLEGRO_LIB) -lalleg
	else
		export ___ALLEGROLDFLAGS := -Llib -lalleg
	endif
endif

___INTOBJCODE := b
___INTCFLAGS  := ${___ALLEGROCFLAGS}
___INTLDFLAGS := ${___ALLEGROLDFLAGS}

