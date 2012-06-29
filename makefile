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

# If CXX is not set, then set it to g++
ifeq (,$(CXX))
	export CXX := g++
endif

ifndef AR
	export AR := ar
endif

# Check if $(WINDRES) is set (resource compiler)
ifndef WINDRES
	export WINDRES := windres
endif

# Use toolchain prefix on default $(CC)
ifneq (,$(TOOLPREFIX))
	export __INT_CC := $(strip $(TOOLPREFIX))
	export __INT_CXX := $(strip $(TOOLPREFIX))
	export __INT_WINDRES := $(strip $(TOOLPREFIX))
	export __INT_AR := $(strip $(TOOLPREFIX))
endif
export __INT_CC := $(__INT_CC)$(CC)
export __INT_CXX := $(__INT_CXX)$(CXX)
export __INT_WINDRES := $(__INT_WINDRES)$(WINDRES)
export __INT_AR := $(__INT_AR)$(AR)

# Have a prefix for the host's GCC
ifneq (,$(HOSTPREFIX))
	export __INT_HOSTPREFIX := $(strip $(HOSTPREFIX))
endif
export __INT_HOSTCC := $(__INT_HOSTPREFIX)gcc

# Build Base (For Out of tree builds */
export __INT_OUTOFTREE := $(OUTOFTREE)

# Extra Options for CDefs
export __INT_CDEFS := $(CDEFS)

#####################
### COMPILE FLAGS ###
#####################

__INT_COMMONCFLAGS  := -fno-exceptions -fno-strict-aliasing -D__STDC_LIMIT_MACROS -D__STDC_CONSTANT_MACROS
__INT_COMMONLDFLAGS := -fno-exceptions -static-libgcc $(strip $(shell $(__INT_CXX) -print-file-name=libstdc++.a))

# Debugging?
ifdef DEBUG
	__INT_MCFLAGS := -g3 -O0 -D_DEBUG -Wall
	__INT_MLDFLAGS := -g3 -O0
	__INT_RCFLAGS := -D_DEBUG
	export __INT_OBJPREFIX := d
	export __INT_EXESUFFIX := -dbg
else
	__INT_MCFLAGS := -g0 -O2 -DNDEBUG
	__INT_MLDFLAGS := -g0 -O2
	__INT_RCFLAGS := -DNDEBUG
	export __INT_OBJPREFIX := r
	export __INT_EXESUFFIX :=
endif

# CC Flags
export __INT_CFLAGS := $(CFLAGS) $(__INT_COMMONCFLAGS) $(__INT_MCFLAGS)

# Linker Flags
export __INT_LDFLAGS := $(LDFLAGS) $(__INT_COMMONLDFLAGS) $(__INT_MLDFLAGS)

# Interface?
export __INT_INTERFACE := $(USEINTERFACE)

##################################
### HOW DO WE EXECUTE COMMANDS ###
##################################

# Ugly lowercasing for shells
__INT_LOWERJUNK = $(subst A,a,$(subst B,b,$(subst C,c,$(subst D,d,$(subst E,e,$(subst F,f,$(subst G,g,$(subst H,h,$(subst I,i,$(subst J,j,$(subst K,k,$(subst L,l,$(subst M,m,$(subst N,n,$(subst O,o,$(subst P,p,$(subst Q,q,$(subst R,r,$(subst S,s,$(subst T,t,$(subst U,u,$(subst V,v,$(subst W,w,$(subst X,x,$(subst Y,y,$(subst Z,z,$1))))))))))))))))))))))))))

# Do lowercasing
__INT_LSHELL   := $(call __INT_LOWERJUNK,$(SHELL))
__INT_LCOMSPEC := $(call __INT_LOWERJUNK,$(COMSPEC))

# When using GNUWin32's make on Windows, it seems COMSPEC is NOT passed to
# make when it should be. Therefor, set COMPSPEC since ths shell must be known.
ifeq (,$(strip $(__INT_LCOMSPEC)))
	__INT_HCOMSPEC := $(strip $(shell ____spec))
	__INT_LCOMSPEC := $(call __INT_LOWERJUNK,$(__INT_HCOMSPEC))
	
	ifneq (,$(strip $(__INT_LCOMSPEC)))
		__INT_CHECKREALSH := yes
	endif
endif

# Another thing to check on GNUWin32 is that SHELL is set to sh.exe (which in
# most cases, does not exist). A simple case is to check whether a command
# actually prints what I want it to print.
ifeq (yes,$(strip $(__INT_CHECKREALSH)))
	__INT_CHECKSHELL := $(call __INT_LOWERJUNK,$(strip $(shell $(SHELL) echo hello)))
	
	# If it does not say hello then it does not exist
	ifneq (hello,$(strip $(__INT_CHECKSHELL)))
		__INT_LSHELL := $(__INT_LCOMSPEC)
		export COMSPEC := $(__INT_HCOMSPEC)
	endif
endif

# Check if COMSPEC is set, if it is, we are on crippled WinDOS
#  GNU Make always sets $(SHELL) to something
#    DOS    : COMSPEC should match SHELL (for DJGPP at least)
#    CygWin : SHELL == /bin/sh, COMPSPEC = command (Use shell here)
#    Windows: COMPSEC should match SHELL
ifeq ($(strip $(__INT_LSHELL)),$(strip $(__INT_LCOMSPEC)))
	__INT_RUNCURDIR  = 
	__INT_DELETE     = del
	__INT_RUNCOMMAND = $(COMSPEC) /C $1
	export __INT_ISONDOS = yes

# Otherwise assume a UNIX shell, or at least a compatible one
else
	__INT_RUNCURDIR  = ./
	__INT_DELETE     = rm -f
	__INT_RUNCOMMAND = $(SHELL) -c "$1"
	export __INT_ISONDOS = no

#
endif

#####################
### LIBRARY STUFF ###
#####################

### SDL ###

# Use sdl-config if SDL_LIB and SDL_INCLUDE are not set
ifeq (,$(strip $(SDL_LIB))$(strip $(SDL_INCLUDE)))
	ifneq (,$(findstring 1.2,$(strip $(shell $(call __INT_RUNCOMMAND,sdl-config --version)))))
		export __INT_SDLCFLAGS  := $(shell $(call __INT_RUNCOMMAND,sdl-config --cflags))
		export __INT_SDLLDFLAGS := $(shell $(call __INT_RUNCOMMAND,sdl-config --libs))
	endif
endif

# Fall back to environment variables
ifeq (,$(strip $(__INT_SDLCFLAGS)))
	ifneq (,$(strip $(SDL_INCLUDE)))
		export __INT_SDLCFLAGS  := -I$(SDL_INCLUDE)
	else
		export __INT_SDLCFLAGS  := -Iinclude -ISDL
	endif
endif

ifeq (,$(strip $(__INT_SDLLDFLAGS)))
	ifneq (,$(strip $(SDL_LIB)))
		export __INT_SDLLDFLAGS := -L$(SDL_LIB) -lSDLmain -lSDL
	else
		export __INT_SDLLDFLAGS := -Llib -lSDLmain -lSDL
	endif
endif

### ALLEGRO ###

# Use allegro-config if ALLEGRO_LIB and ALLEGRO_INCLUDE are not set
ifeq (,$(strip $(ALLEGRO_LIB))$(strip $(ALLEGRO_INCLUDE)))
	ifneq (,$(findstring 4.2,$(strip $(shell $(call __INT_RUNCOMMAND,allegro-config --version)))))
		export __INT_ALLEGROCFLAGS  := $(shell $(call __INT_RUNCOMMAND,allegro-config --cflags))
		export __INT_ALLEGROLDFLAGS := $(shell $(call __INT_RUNCOMMAND,allegro-config --libs))
	endif
endif

# Fall back to environment variables
ifeq (,$(strip $(__INT_ALLEGROCFLAGS)))
	ifneq (,$(strip $(ALLEGRO_INCLUDE)))
		export __INT_ALLEGROCFLAGS  := -I$(ALLEGRO_INCLUDE)
	else
		export __INT_ALLEGROCFLAGS  := -Iinclude -Iallegro
	endif
endif

ifeq (,$(strip $(__INT_ALLEGROLDFLAGS)))
	ifneq (,$(strip $(ALLEGRO_LIB)))
		export __INT_ALLEGROLDFLAGS := -L$(ALLEGRO_LIB) -lalleg
	else
		export __INT_ALLEGROLDFLAGS := -Llib -lalleg
	endif
endif

###############
### TARGETS ###
##############/

# Object Location
export __INT_BIN := bin
export __INT_OBJ := objs

# Default
.PHONY: all
all:			wad $(__INT_OBJ) ____make.___
				@$(MAKE) -f ____make.___ all

# Clean
clean:			$(__INT_OBJ) ____make.___
				@$(MAKE) -f ____make.___ clean

# The game
remood:			wad $(__INT_OBJ) ____make.___
				@$(MAKE) -f ____make.___ remood

# Object directory
$(__INT_OBJ):	
				$(__INT_RUNCOMMAND,mkdir $@)

# ReMooD.WAD
$(__INT_BIN)/remood.wad:	rmdtext.exe $(__INT_OUTOFTREE)wad/wadinfo.txt
							@$(call __INT_RUNCOMMAND,$(__INT_RUNCURDIR)rmdtext.exe $(__INT_OUTOFTREE)wad/wadinfo.txt $@ $(__INT_OUTOFTREE)wad/)

wad:						bin/remood.wad

###################################
### MAGICAL MAKEFILE GENERATION ###
###################################

# Phony WAD Target
.PHONY:
wad/wadinfo.txt:	
					

# Preprocessor for environment
.PHONY: ____cdef.___
____cdef.___:	$(__INT_OUTOFTREE)version
				@$(__INT_CC) $(__INT_CDEFS) -E -x c -dM $< -o $@

# Program that builds the makefile (always rebuild)
# It is suffixed with exe for DOS, UNIX could care less
.PHONY: ____bild.exe
____bild.exe:	$(__INT_OUTOFTREE)____makc.___ ____cdef.___
				@$(__INT_HOSTCC) -o $@ -x c $<

# The actual makefile, generated by ____bild.exe
____make.___:	____bild.exe
				@$(call __INT_RUNCOMMAND,$(__INT_RUNCURDIR)____bild.exe ____cdef.___ $(__INT_OUTOFTREE)____mtmp.___ $@)

# RMDTEX -- DeuTex Clone For ReMooD
rmdtext.exe:	$(__INT_OUTOFTREE)util/rmdtex.c
				@$(__INT_HOSTCC) -o $@ $<


