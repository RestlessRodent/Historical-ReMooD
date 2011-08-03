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
# Another makefile setup, dynamic recursive makes
# Root makefile, calls other makefiles
# Lean and mean!

################
### COMPILER ###
################

# If $(CC) is cc, make it gcc!
ifndef KEEPCC
	ifeq (cc,$(CC))
		export CC := gcc
	endif
endif

# Use toolchain prefix on default $(CC)
ifneq (,$(TOOLPREFIX))
	export __INT_CC := $(TOOLPREFIX)
endif
export __INT_CC := $(__INT_CC)$(CC)

#####################
### COMPILE FLAGS ###
#####################

# Debugging?
ifdef DEBUG
	__INT_MCFLAGS := -g3 -O0
	__INT_MLDFLAGS := -g3 -O0
	export __INT_OBJPREFIX := d
	export __INT_EXESUFFIX := -dbg
else
	__INT_MCFLAGS := -g0 -O2
	__INT_MLDFLAGS := -g0 -O2
	export __INT_OBJPREFIX := r
	export __INT_EXESUFFIX :=
endif

# CC Flags
export __INT_CFLAGS := $(CFLAGS) $(__INT_MCFLAGS)

# Linker Flags
export __INT_LDFLAGS := $(LDFLAGS) $(__INT_MLDFLAGS)

##################################
### HOW DO WE EXECUTE COMMANDS ###
#################################/

# Ugly lowercasing for shells
__INT_LOWERJUNK = $(subst A,a,$(subst B,b,$(subst C,c,$(subst D,d,$(subst E,e,$(subst F,f,$(subst G,g,$(subst H,h,$(subst I,i,$(subst J,j,$(subst K,k,$(subst L,l,$(subst M,m,$(subst N,n,$(subst O,o,$(subst P,p,$(subst Q,q,$(subst R,r,$(subst S,s,$(subst T,t,$(subst U,u,$(subst V,v,$(subst W,w,$(subst X,x,$(subst Y,y,$(subst Z,z,$1))))))))))))))))))))))))))

# Do lowercasing
__INT_LSHELL   := $(call __INT_LOWERJUNK,$(SHELL))
__INT_LCOMSPEC := $(call __INT_LOWERJUNK,$(COMSPEC))

# Check if COMSPEC is set, if it we, are are on crippled WinDOS
#  GNU Make always sets $(SHELL) to something
#    DOS    : COMSPEC should match SHELL (for DJGPP at least)
#    CygWin : SHELL == /bin/sh, COMPSPEC = command (Use shell here)
#    Windows: COMPSEC should match SHELL
ifeq ($(strip $(__INT_LSHELL)),$(strip $(__INT_LCOMSPEC)))
	__INT_RUNCURDIR  = 
	__INT_DELETE     = del
	__INT_RUNCOMMAND = $(COMSPEC) /C $1

# Otherwise assume a UNIX shell, or at least a compatible on
else
	__INT_RUNCURDIR  = ./
	__INT_DELETE     = rm -f
	__INT_RUNCOMMAND = $(SHELL) -c "$1"

#
endif

###########
### SDL ###
###########

# Use sdl-config if SDL_LIB and SDL_INCLUDE are not set
ifeq (,$(strip $(SDL_LIB))$(strip $(SDL_INCLUDE)))
	ifneq (,$(strip $(call __INT_RUNCOMMAND,sdl-config --version)))
		export __INT_SDLCFLAGS  := $(shell $(call __INT_RUNCOMMAND,sdl-config --cflags))
		export __INT_SDLLDFLAGS := $(shell $(call __INT_RUNCOMMAND,sdl-config --libs))
	# Not found so make some assumptions
	else
$(warning SDL Config not found)
		export __INT_SDLCFLAGS  := -ISDL -Iinclude
		export __INT_SDLLDFLAGS := -lSDL -Llib
	endif
# Otherwise use them instead
else
	ifneq (,$(strip $(SDL_INCLUDE)))
		export __INT_SDLCFLAGS  := -I$(SDL_INCLUDE)
	else
		export __INT_SDLCFLAGS  := -Iinclude
	endif
	
	ifneq (,$(strip $(SDL_LIB)))
		export __INT_SDLLDFLAGS := -L$(SDL_LIB) -lSDL
	else
		export __INT_SDLLDFLAGS :=  -Llib -lSDL
	endif
endif

###############
### TARGETS ###
##############/

# Object Location
export __INT_OBJ := objs

# Default
.PHONY: all
all:			$(__INT_OBJ) ____make.___
				@$(MAKE) -f ____make.___ all

# Clean
clean:			$(__INT_OBJ) ____make.___
				@$(MAKE) -f ____make.___ clean

# The game
remood:			$(__INT_OBJ) ____make.___
				@$(MAKE) -f ____make.___ remood

# Object directory
$(__INT_OBJ):	
				$(__INT_RUNCOMMAND,mkdir $@)

###################################
### MAGICAL MAKEFILE GENERATION ###
###################################

# Preprocessor for environment
.PHONY: ____cdef.___
____cdef.___:	version
				@$(__INT_CC) -E -x c -dM $< -o $@

# Program that builds the makefile (always rebuild)
# It is suffixed with exe for DOS, UNIX could care less
.PHONY: ____bild.exe
____bild.exe:	____makc.___ ____cdef.___
				@gcc -o $@ -x c $<

# The actual makefile, generated by ____bild.exe
____make.___:	____bild.exe
				@$(call __INT_RUNCOMMAND,$(__INT_RUNCURDIR)____bild.exe ____cdef.___ ____mtmp.___ $@)

