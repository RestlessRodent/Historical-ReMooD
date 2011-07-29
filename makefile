# ########   ###### #####   #####  ######   ######  ######
# ##     ##  ##     ##  ## ##  ## ##    ## ##    ## ##   ##
# ##     ##  ##     ##   ###   ## ##    ## ##    ## ##    ##
# ########   ####   ##    #    ## ##    ## ##    ## ##    ##
# ##    ##   ##     ##         ## ##    ## ##    ## ##    ##
# ##     ##  ##     ##         ## ##    ## ##    ## ##   ##
# ##      ## ###### ##         ##  ######   ######  ######
#                      http://remood.org/
# -----------------------------------------------------------------------------
# Copyright (C) 2009-2011 GhostlyDeath <ghostlydeath@remood.org>
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
# Another makefile setup, recursive makes
# Root makefile, calls other makefiles

################
### COMPILER ###
################

# Use toolchain prefix on default $(CC)
ifneq (,$(TOOLPREFIX))
	export __INT_CC := $(TOOLPREFIX)
endif
export __INT_CC := $(__INT_CC)$(CC)

##########################
### MAKEFILE SELECTION ###
##########################

# Which makefile do we use?
USEMAKEFILE ?= auto

# If we selected "auto" then we detect based on the host
ifeq (auto,$(strip $(USEMAKEFILE)))
	ifneq (,$(findstring linux,$(strip $(shell uname -s))))
		USEMAKEFILE := lnx
	else
		ifneq (,$(findstring Linux,$(strip $(shell uname -s))))
			USEMAKEFILE := lnx
		else
			ifneq (,$(findstring FreeBSD,$(strip $(shell uname -s))))
				USEMAKEFILE := bsd
			else
				ifneq (,$(findstring Darwin,$(strip $(shell uname -s))))
					USEMAKEFILE := mox
				else
					ifneq (,$(findstring Windows,$(strip $(shell pr_ver.bat))))
						ifneq (,$(findstring AMD64,$(strip $(PROCESSOR_ARCHITECTURE))))
							USEMAKEFILE := w64
						else
							USEMAKEFILE := w32
						endif
					else
						ifneq (,$(findstring ReactOS,$(strip $(shell pr_ver.bat))))
							USEMAKEFILE := w32
						else
							# Real MS-DOS or DOSBOX
								# DOSBox version 0.74, Reported DOS version 5.0
							ifneq (,$(findstring DOS,$(strip $(shell pr_ver.bat))))
								USEMAKEFILE := djd
							else
								# WINE
								ifneq (,$(findstring CMD,$(strip $(shell pr_ver.bat))))
									USEMAKEFILE := w32
								else
									# DOSEmu
									ifneq (,$(findstring DOS,$(strip $(shell "pr_ver.bat /r"))))
										USEMAKEFILE := djd
									endif
								endif
							endif
						endif
					endif
				endif
			endif
		endif
	endif
endif

########################
### INTERFACE TO USE ###
########################

# Which interface do we use?
USEINTERFACE ?= auto

# Based on the makefile we are using...
ifeq (auto,$(USEINTERFACE))
	ifeq (lnx,$(USEMAKEFILE))
		USEINTERFACE := sdl
	else
		ifeq (bsd,$(USEMAKEFILE))
			USEINTERFACE := sdl
		else
			ifeq (mox,$(USEMAKEFILE))
				USEINTERFACE := sdl
			else
				ifeq (w32,$(USEMAKEFILE))
					USEINTERFACE := sdl
				else
					ifeq (w64,$(USEMAKEFILE))
						USEINTERFACE := sdl
					else
						ifeq (djd,$(USEMAKEFILE))
							USEINTERFACE := alg
						endif
					endif
				endif
			endif
		endif
	endif
endif

# Slap onto internal
export __INT_INTERFACE := $(USEINTERFACE)

#####################
### COMPILE FLAGS ###
#####################

# Debugging?
ifdef DEBUG
	__INT_MCFLAGS := -g3 -O0
	__INT_MLDFLAGS := -g3 -O0
	export __INT_OBJPREFIX := d
else
	__INT_MCFLAGS := -g0 -O2
	__INT_MLDFLAGS := -g0 -O2
	export __INT_OBJPREFIX := r
endif

# CC Flags
export __INT_CFLAGS := $(CFLAGS) $(__INT_MCFLAGS)

# Linker Flags
export __INT_LDFLAGS := $(LDFLAGS) $(__INT_MLDFLAGS)

###############
### TARGETS ###
##############/

all:			
				$(MAKE) -f makefile.$(USEMAKEFILE) all

clean:			
				$(MAKE) -f makefile.$(USEMAKEFILE) clean

remood:			
				$(MAKE) -f makefile.$(USEMAKEFILE) remood

