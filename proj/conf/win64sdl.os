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
# Compiles for Windows 64-bit with SDL as the interface

####################
### PRECONFIGURE ###
####################

### IF TOOLPREFIX IS SET, ASSUME AVAILABLE ###
ifdef __WIN64_TOOLPREFIX
	# Download the Win64 SDL Binary if needed
    __WIN64_SDLBIN := $(shell ${__MASTER_PROJ}/conf/dlxtract.sh "win64sdl" "${__MASTER_ROOT}")
    
    # Check to see if SDL works with this compiler
    ifeq (${__WIN64_SDLBIN},ok)
        $(info +++ Win32 SDL Downloaded)
        
        __WIN64_CHECKSDL := $(shell ${__MASTER_PROJ}/conf/check.sh "win64sdl")
    endif
    
    # Add to the list, if the target worked
    ifeq (${__WIN64_CHECKSDL},ok)
        $(info +++ Win32 SDL OK)
    	__MASTER_OKTARGETS := win64sdl ${__MASTER_OKTARGETS}
    endif
endif

#############
### RULES ###
#############

__WIN64SDL_BASE := make -C /tmp/remood-win64sdl -f ${__MASTER_ROOT}/makefile OS=win32 INTERFACE=sdl TOOLPREFIX=${__WIN32_TOOLPREFIX} SDL_INCLUDE=${__MASTER_ROOT}/win64sdl/include/SDL SDL_LIB=${__MASTER_ROOT}/win64sdl/lib

### PREPARE BUILD ###
_win64sdl_prep:			
						mkdir -p /tmp/remood-win64sdl /tmp/remood-win64sdl/bin /tmp/remood-win64sdl/o /tmp/remood-win64sdl/o/c /tmp/remood-win64sdl/o/s

### PERFORM BUILD ###
_win64sdl_build:		_win64sdl_prep
						${__WIN64SDL_BASE} clean
						${__WIN64SDL_BASE}

### ARCHIVE FILES ###
_win64sdl_archive:		
						@:


