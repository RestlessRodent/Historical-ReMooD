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
# Operating System Detection (Build Target)

#####################################
### CURRENT TARGET GCC BEING USED ###
#####################################
# This can get ugly especially if a new target appears that is not known about.

___GCCMACH := $(shell $(call ___RUNCOMMAND,$(___CC) -dumpmachine))
___SUBS    := $(subst -,${___SPACE} ${___SPACE},${___GCCMACH})
___FINDMACH = $(strip $(foreach __x,$1 $2 $3 $4 $5 $6 $7 $8 $9,$(findstring ${__x},${___SUBS})))

###############################
### PASSED VIA COMMAND LINE ###
###############################

# Force lowercase
ifneq (,$(strip ${OS}))
___OS=$(call ___LOWERJUNK,$(strip ${OS}))
endif

#############
### LINUX ###
#############
# *linux*

ifeq (,${___OS})
ifneq (,$(call ___FINDMACH,linux))

___OS=linux

endif
endif

###########
### DOS ###
###########
# djgpp

ifeq (,${___OS})
ifneq (,$(call ___FINDMACH,djgpp))

___OS=dos

endif
endif

###############
### PALM OS ###
###############
# *palmos*

ifeq (,${___OS})
ifneq (,$(call ___FINDMACH,palmos))

___OS=palmos

endif
endif

################
### MAC OS X ###
################
# ???

ifeq (,${___OS})

#___OS=macosx

endif

######################
### MAC OS CLASSIC ###
######################
# ???

ifeq (,${___OS})

#___OS=macos

endif

#############
### AMIGA ###
#############
# ???

ifeq (,${___OS})

#___OS=amiga

endif

###############
### SOLARIS ###
###############
# ???

ifeq (,${___OS})

#___OS=solaris

endif

####################
### GAMECUBE/WII ###
####################
# ???

ifeq (,${___OS})

#___OS=gcwii

endif

##################
### WINDOWS CE ###
##################
# ???

ifeq (,${___OS})
ifneq (,$(call ___FINDMACH,mingw32))
ifneq (,$(call ___FINDMACH,cegcc))

___OS=wince

endif
endif
endif

######################
### WINDOWS 64-BIT ###
######################
# x86_64-w64-mingw32

ifeq (,${___OS})
ifneq (,$(call ___FINDMACH,mingw32))
ifneq (,$(call ___FINDMACH,x86_64 amd64))

___OS=win64

endif
endif
endif

######################
### WINDOWS 32-BIT ###
######################
# i686-w64-mingw32

ifeq (,${___OS})
ifneq (,$(call ___FINDMACH,mingw32 mingw32msvc))
ifneq (,$(call ___FINDMACH,i386 i486 i586 i686))

___OS=win32

endif
endif
endif

