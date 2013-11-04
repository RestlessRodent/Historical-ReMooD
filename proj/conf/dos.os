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
# Compiles for DOS either using a native DJGPP compiler or via DOSEMU (with DJ)
# Note that, I use DOSEMU for a prepopulated DJGPP setup because I could never
# build a DJGPP cross compiler on Linux. The compiler always segfaults but works
# perfectly fine for DOS itself. In the future, it is quite possible to even
# compile using DOSBox (although it will be much more slower than DOSEMU) so
# that builds can be done on systems where DOSEMU would not otherwise work.
# Some may say that compiling for DOS is not possible, but you can compile for
# DOS on a DOS system (maybe you can even install mingw-w64 on DJGGP).
#
# For DOSEMU:
# * To work properly, install DJGPP for DOS as per usual instructions by looking
#   at http://www.delorie.com/djgpp/
# * When it gets time to add the environment variables to AUTOEXEC.BAT, be sure
#   that they are added before the line that says "unix -e", otherwise the
#   environment for DJGPP will not be set, and the commands will fail.
#
# This DOS target only supports allegro.

####################
### PRECONFIGURE ###
####################

### TRY TO FIND THE NATIVE COMPILER FIRST ###

# Host compiler IS the DOS compiler
ifeq ($(findstring msdosdjgpp,${__MASTER_HOST}),msdosdjgpp)
    __DOS_PATHOFNDJ := gcc
endif

# Try using shell commands to find it
ifndef __DOS_PATHOFNDJ
    __DOS_I586 := $(shell which i586-pc-msdosdjgpp-gcc)
    __DOS_I486 := $(shell which i486-pc-msdosdjgpp-gcc)
    __DOS_I386 := $(shell which i386-pc-msdosdjgpp-gcc)
    
    ifneq (${__DOS_I586},)
        __DOS_PATHOFNDJ := ${__DOS_I586}
    else
        ifneq (${__DOS_I486},)
            __DOS_PATHOFNDJ := ${__DOS_I486}
        else
            ifneq (${__DOS_I386},)
                __DOS_PATHOFNDJ := ${__DOS_I386}
            endif
        endif
    endif
endif

# Information on DOS native compiler
ifdef __DOS_PATHOFNDJ
    # Print a nice message
    $(info +++ Found native DOS compiler at ${__DOS_PATHOFNDJ})
    
    # Now see if it is possible to work with allegro.
    # Normally allegro has allegro-config, but for DOS, that really does not
    # matter much.
    __DOS_CHECKALLEGRO := $(shell ${__MASTER_PROJ}/conf/djtest.sh "${__DOS_PATHOFNDJ}" "${__MASTER_ROOT}")
    
    # Add to the list, if the target worked
    ifeq (${__DOS_CHECKALLEGRO},ok)
    	__MASTER_OKTARGETS := dos ${__MASTER_OKTARGETS}
    	__DOS_USENATIVE := true
    endif
else
    $(info +++ No native DOS compiler found)
    
    # Since we do not have DJGPP on the host system, rely on using DOSEMU, if
    # that even exists
    ifneq ($(strip ${__MASTER_DOSEMUPATH}),)
    	__DOS_TRYDOSEMU := true
    endif
endif

# Trying DOSEMU?
ifeq (${__DOS_TRYDOSEMU},true)
    $(info +++ Attempting to use DOSEMU)
    
    # This will check to see if DJGPP is setup with allegro by running a command
    # through DOSEMU and seeing if it exists in the output.
    __DOS_CHECKDOSEMU := $(shell ${__MASTER_PROJ}/conf/detest.sh "${__MASTER_DOSEMUPATH}" "${__MASTER_ROOT}")
    
    # Add to the list, if the target worked
    ifeq (${__DOS_CHECKDOSEMU},ok)
    	__MASTER_OKTARGETS := dos ${__MASTER_OKTARGETS}
    	__DOS_USEDOSEMU := true
    endif
endif

#####################
### WRAPPED RULES ###
#####################
# The build system for DOS requires this, because it can either use a native
# compiler or the compiler in DOSEMU/DOSBox.

ifdef __DOS_USEDOSEMU
	__DOS_USERULE := dosemu
else
	ifneq (,${__DOS_PATHOFNDJ})
		__DOS_USERULE := native
	endif
endif 

### NATIVE ###

# Prepare Environment
_dos_prep_dosemu:		
						@:

# Do build
.PHONY: _dos_build_dosemu
_dos_build_dosemu:		
						${__MASTER_PROJ}/conf/demub.sh "${__MASTER_DOSEMUPATH}" "${__MASTER_ROOT}"

### DOSEMU ###

#############
### RULES ###
#############

### PREPARE BUILD ###
.PHONY: _dos_prep
_dos_prep:				_dos_prep_${__DOS_USERULE}
						@:

### PERFORM BUILD ###
.PHONY: _dos_build
_dos_build:				_dos_build_${__DOS_USERULE}
						@:

### ARCHIVE FILES ###
_dos_archive:			
						@:

