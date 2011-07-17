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

##########################
### MAKEFILE SELECTION ###
##########################

# Which makefile do we use?
USEMAKEFILE ?= lnx#auto

# If we selected "auto" then we detect based on the host

################
### COMPILER ###
################

# Use toolchain prefix on default $(CC)
ifneq (,$(TOOLPREFIX))
	export __INT_CC := $(TOOLPREFIX)$(CC)
endif
export __INT_CC += $(CC)

###############
### TARGETS ###
##############/

all:			
				$(MAKE) -f makefile.$(USEMAKEFILE) all

clean:			
				$(MAKE) -f makefile.$(USEMAKEFILE) clean

