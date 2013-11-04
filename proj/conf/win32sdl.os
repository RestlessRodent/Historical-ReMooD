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
# Compiles for Windows 32-bit with SDL as the interface

####################
### PRECONFIGURE ###
####################

### IF TOOLPREFIX IS SET, ASSUME AVAILABLE ###
ifdef __WIN32_TOOLPREFIX
	# Download the Win32 SDL Binary if needed
    __WIN32_SDLBIN := $(shell ${__MASTER_PROJ}/conf/dlxtract.sh "win32sdl" "${__MASTER_ROOT}")
    
    # Check to see if SDL works with this compiler
    ifeq (${__WIN32_SDLBIN},ok)
        $(info +++ Win32 SDL Downloaded)
        
        __WIN32_CHECKSDL := $(shell ${__MASTER_PROJ}/conf/check.sh "win32sdl")
    endif
    
    # Add to the list, if the target worked
    ifeq (${__WIN32_CHECKSDL},ok)
        $(info +++ Win32 SDL OK)
    	__MASTER_OKTARGETS := win32sdl ${__MASTER_OKTARGETS}
    endif
endif

#############
### RULES ###
#############

__WIN32SDL_BASE := make -C /tmp/remood-win32sdl -f ${__MASTER_ROOT}/makefile OS=win32 INTERFACE=sdl TOOLPREFIX=${__WIN32_TOOLPREFIX} SDL_INCLUDE=${__MASTER_ROOT}/win32sdl/include/SDL SDL_LIB=${__MASTER_ROOT}/win32sdl/lib

### PREPARE BUILD ###
_win32sdl_prep:			
						mkdir -p /tmp/remood-win32sdl /tmp/remood-win32sdl/bin /tmp/remood-win32sdl/o /tmp/remood-win32sdl/o/c /tmp/remood-win32sdl/o/s

### PERFORM BUILD ###
_win32sdl_build:		_win32sdl_prep
						${__WIN32SDL_BASE} clean
						${__WIN32SDL_BASE}

### ARCHIVE FILES ###
_win32sdl_archive:		
						@:


