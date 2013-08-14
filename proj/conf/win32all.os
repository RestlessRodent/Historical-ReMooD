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
# Compiles for Windows 32-bit with Allegro as the interface
#
# Since Win32 is shared by multiple configs, the Win32 Support is used.

####################
### PRECONFIGURE ###
####################

### IF TOOLPREFIX IS SET, ASSUME AVAILABLE ###
ifdef __WIN32_TOOLPREFIX
	# Download the Win32 Allegro Binary if needed
    __WIN32_ALLEGROBIN := $(shell ${__MASTER_PROJ}/conf/dlxtract.sh "win32all" "${__MASTER_ROOT}")
    
    # Check to see if allegro works with this compiler
    ifeq (${__WIN32_ALLEGROBIN},ok)
        $(info +++ Win32 Allegro Downloaded)
    endif
    
    # Add to the list, if the target worked
    ifeq (${__WIN32_CHECKALLEGRO},ok)
        $(info +++ Win32 Allegro OK)
    	__MASTER_OKTARGETS := win32all ${__MASTER_OKTARGETS}
    endif
endif


