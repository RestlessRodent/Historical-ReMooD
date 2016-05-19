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
# Copyright (C) 2009-2013 GhostlyDeath <ghostlydeath@remood.org>
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
# A third makefile setup, even meaner and even leaner!
# Might not work on ancient Win98 systems using ancient Dev-C++'s GNU Make.

##########################
### CURRENT BUILD ROOT ###
##########################

# Directory where make is
__WORDCOUNT := $(words $(MAKEFILE_LIST))

# If this is zero, then just use current dir
ifeq (0, ${__WORDCOUNT})

__BUILDROOT := .

else

__BUILDROOT	:= $(subst //,/,$(dir $(word ${__WORDCOUNT},$(MAKEFILE_LIST))))

endif

##################################
### HOW DO WE EXECUTE COMMANDS ###
##################################
# This is here so that the makefile can still execute certain commands on DOS.

# Ugly lowercasing for shells
___LOWERJUNK = $(subst A,a,$(subst B,b,$(subst C,c,$(subst D,d,$(subst E,e,$(subst F,f,$(subst G,g,$(subst H,h,$(subst I,i,$(subst J,j,$(subst K,k,$(subst L,l,$(subst M,m,$(subst N,n,$(subst O,o,$(subst P,p,$(subst Q,q,$(subst R,r,$(subst S,s,$(subst T,t,$(subst U,u,$(subst V,v,$(subst W,w,$(subst X,x,$(subst Y,y,$(subst Z,z,$1))))))))))))))))))))))))))

# Do lowercasing
___LSHELL   := $(call ___LOWERJUNK,$(SHELL))
___LCOMSPEC := $(call ___LOWERJUNK,$(COMSPEC))

# When using GNUWin32's make on Windows, it seems COMSPEC is NOT passed to
# make when it should be. Therefor, set COMPSPEC since ths shell must be known.
ifeq (,$(strip $(___LCOMSPEC)))
	# This produces an annoying: `make: ____spec: Command not found`
	___HCOMSPEC := $(strip $(shell ____spec))
	___LCOMSPEC := $(call ___LOWERJUNK,$(___HCOMSPEC))
	
	ifneq (,$(strip $(___LCOMSPEC)))
		___CHECKREALSH := yes
	endif
endif

# Another thing to check on GNUWin32 is that SHELL is set to sh.exe (which in
# most cases, does not exist). A simple case is to check whether a command
# actually prints what I want it to print.
ifeq (yes,$(strip $(___CHECKREALSH)))
	___CHECKSHELL := $(call ___LOWERJUNK,$(strip $(shell $(SHELL) echo hello)))
	
	# If it does not say hello then it does not exist
	ifneq (hello,$(strip $(___CHECKSHELL)))
		___LSHELL := $(___LCOMSPEC)
		export COMSPEC := $(___HCOMSPEC)
	endif
endif

# Check if COMSPEC is set, if it is, we are on crippled WinDOS
#  GNU Make always sets $(SHELL) to something
#    DOS    : COMSPEC should match SHELL (for DJGPP at least)
#    CygWin : SHELL == /bin/sh, COMPSPEC = command (Use shell here)
#    Windows: COMPSEC should match SHELL
ifeq ($(strip $(___LSHELL)),$(strip $(___LCOMSPEC)))
	___RUNCURDIR  = 
	___DELETE     = del
	___RUNCOMMAND = $(COMSPEC) /C $1
	___DIRSEP     := $(strip \ )
	___DOSPATH    = $(subst /,\,$1)
	___DEVNULL	:= NUL
	___ISONDOS = yes

# Otherwise assume a UNIX shell, or at least a compatible one
else
	___RUNCURDIR  = ./
	___DELETE     = rm -f
	___RUNCOMMAND = $(SHELL) -c "$1"
	___ISONDOS = no
	___DIRSEP     := /
	___DOSPATH    = $1
	___DEVNULL    := /dev/null

#
endif

#############################
### PREFIX AND C COMPILER ###
#############################

# If $(CC) is cc, make it gcc!
ifndef KEEPCC
ifeq (cc,$(CC))

export CC := gcc

endif
endif

# Makes libraries
ifndef AR

export AR := ar

endif

# Check if $(WINDRES) is set (resource compiler)
ifndef WINDRES

export WINDRES := windres

endif

# Use toolchain prefix on default $(CC)
ifneq (,$(TOOLPREFIX))

export ___CC := $(strip $(TOOLPREFIX))
export ___WINDRES := $(strip $(TOOLPREFIX))
export ___AR := $(strip $(TOOLPREFIX))

endif

export ___CC := $(___CC)$(CC)
export ___WINDRES := $(___WINDRES)$(WINDRES)
export ___AR := $(___AR)$(AR)

# Have a prefix for the host's GCC
ifneq (,$(HOSTPREFIX))

export ___HOSTPREFIX := $(strip $(HOSTPREFIX))

endif
export ___HOSTCC := $(___HOSTPREFIX)gcc

#################
### DEBUGGING ###
#################

# Enabled or not?
ifdef DEBUG

___DEBUG := yes
-include ${__BUILDROOT}/util/debug.y

else

___DEBUG := no
-include ${__BUILDROOT}/util/debug.n

endif

############
### JAVA ###
############

# Java header not found
ifeq (,${JAVA_INCLUDE})
$(error JAVA_INCLUDE not specified)
endif

# Java library not found
ifeq (,${JAVA_LIB})
$(error JAVA_LIB not specified)
endif

########################
### OPERATING SYSTEM ###
########################

# OS Detection Code (For Target Compiler)
include ${__BUILDROOT}/util/detect.os

# Operating system not found
ifeq (,${___OS})

$(error Operating system could not be detected, use OS=whatever)

else

# Print OS
$(info Target OS: ${___OS})

endif

# Include OS makefile
-include ${__BUILDROOT}/src/os/${___OS}/makefile

##################
### INTERFACES ###
##################

# Every available interface
___ALLINTERFACES := $(foreach __mk,$(wildcard ${__BUILDROOT}/src/plat/*/makefile),$(word 3,$(subst /,${___SPACE} ${___SPACE},${__mk})))

# Specified by command line
ifneq (,$(strip ${INTERFACE}))

___INTERFACE=$(strip ${INTERFACE})

else

# Use default, if specified...
ifneq (,$(strip ${___DEFAULTINTERFACE}))

___INTERFACE=${___DEFAULTINTERFACE}

# Use headless interface, if not specified
else

___INTERFACE=headless

endif

endif

# Print Interface
$(info Interface: ${___INTERFACE})

# Include interface makefile
-include ${__BUILDROOT}/src/plat/${___INTERFACE}/makefile

##############################
### DEFAULT INITIALIZATION ###
##############################

# Set output binary to just remood, if interface has nothing set
ifeq (,$(strip ${___CLEXENAME}))

ifeq (yes,${___DEBUG})
___CLEXENAME := remood-dbg
else
___CLEXENAME := remood
endif

endif

# Same goes for server
ifeq (,$(strip ${___SVEXENAME}))

ifeq (yes,${___DEBUG})
___SVEXENAME := remood-server-dbg
else
___SVEXENAME := remood-server
endif

endif

###########################
### CLIENT/SERVER RULES ###
###########################

# Object Code
___OBJEXT := ${___OSOBJCODE}${___INTOBJCODE}${___DBGOBJCODE}

# Internals
-include ${__BUILDROOT}/src/client/makefile

ifndef ___NOSERVER

-include ${__BUILDROOT}/src/server/makefile

endif

# Shared source files
___SHSRC := $(wildcard ${__BUILDROOT}/src/*.c ${__BUILDROOT}/src/plat/${___INTERFACE}/*.c ${__BUILDROOT}/src/os/${___OS}/*.c)

# Objects for client
___CLSRC := ${___SHSRC} $(wildcard ${__BUILDROOT}/src/client/*.c)
___CLOBJ := $(foreach __c,${___CLSRC},o/c/$(notdir $(basename ${__c}).${___OBJEXT}))
___CLDEL := $(foreach __o,${___CLOBJ},ccl___${__o})

# Objects for server
ifndef ___NOSERVER

___SVSRC := ${___SHSRC} $(wildcard ${__BUILDROOT}/src/server/*.c)
___SVOBJ := $(foreach __c,${___SVSRC},o/s/$(notdir $(basename ${__c}).${___OBJEXT}))
___SVDEL := $(foreach __o,${___SVOBJ},csv___${__o})

endif

# Combined flags
___BASECFLAGS := -I${__BUILDROOT}/src/ -I${__BUILDROOT}/src/plat/${___INTERFACE} -I${__BUILDROOT}/src/os/${___OS} -I${JAVA_INCLUDE} ${CFLAGS}
___BASELDFLAGS := -L${JAVA_LIB} -ljvm ${LDFLAGS}

___SHXCFLAGS  := ${___BASECFLAGS} ${___DBGCFLAGS} ${___OSCFLAGS} ${___INTCFLAGS}
___SHXLDFLAGS := ${___DBGLDFLAGS} ${___OSLDFLAGS} ${___INTLDFLAGS} ${___BASELDFLAGS}

___CLXCFLAGS  := -I${__BUILDROOT}/src/client ${___SHXCFLAGS}
___CLXLDFLAGS := ${___SHXLDFLAGS} ${___BASELDFLAGS}

ifndef ___NOSERVER

___SVXCFLAGS  := -D__REMOOD_DEDICATED -I${__BUILDROOT}/src/server ${___SHXCFLAGS}
___SVXLDFLAGS := ${___SHXLDFLAGS} ${___BASELDFLAGS}

endif

#############
### RULES ###
#############

# Build All
.PHONY: all
all:						client remood.jar
							-@:

# Clean All
.PHONY: clean
clean:						clean-client clean-server
							-@:

# Client
.PHONY: client
client:						bin/${___CLEXENAME} bin/remood.wad
							-@:

# ReMooD Client
bin/${___CLEXENAME}:		${___CLOBJ}
							@echo [LD] $(notdir $@)
							@${___CC} -o $@ ${___CLOBJ} ${___CLXLDFLAGS}

# Versions of make before 3.80 do not support $(eval) =[
o/c/%.${___OBJEXT}:			${__BUILDROOT}/src/%.c
							@echo [CC] $(notdir $<)
							@${___CC} -o $@ -c $< ${___CLXCFLAGS}

o/c/%.${___OBJEXT}:			${__BUILDROOT}/src/client/%.c
							@echo [CC] $(notdir $<)
							@${___CC} -o $@ -c $< ${___CLXCFLAGS}

o/c/%.${___OBJEXT}:			${__BUILDROOT}/src/plat/${___INTERFACE}/%.c
							@echo [CC] $(notdir $<)
							@${___CC} -o $@ -c $< ${___CLXCFLAGS}

o/c/%.${___OBJEXT}:			${__BUILDROOT}/src/os/${___OS}/%.c
							@echo [CC] $(notdir $<)
							@${___CC} -o $@ -c $< ${___CLXCFLAGS}


# Server
ifndef ___NOSERVER

# Generic Target
.PHONY: server
server:						bin/${___SVEXENAME} bin/remood.wad
							-@:

# ReMooD Server
bin/${___SVEXENAME}:		${___SVOBJ}
							@echo [LD] $@
							@${___CC} -o $@ ${___SVOBJ} ${___SVXLDFLAGS}

o/s/%.${___OBJEXT}:			${__BUILDROOT}/src/%.c
							@echo [CC] $(notdir $<)
							@${___CC} -o $@ -c $< ${___SVXCFLAGS}

o/s/%.${___OBJEXT}:			${__BUILDROOT}/src/server/%.c
							@echo [CC] $(notdir $<)
							@${___CC} -o $@ -c $< ${___SVXCFLAGS}

o/s/%.${___OBJEXT}:			${__BUILDROOT}/src/plat/${___INTERFACE}/%.c
							@echo [CC] $(notdir $<)
							@${___CC} -o $@ -c $< ${___SVXCFLAGS}

o/s/%.${___OBJEXT}:			src/os/${___OS}/%.c
							@echo [CC] $(notdir $<)
							@${___CC} -o $@ -c $< ${___SVXCFLAGS}

else

# Generic Target
.PHONY: server
server:						
							-@:

endif

# ReMooD.wad
bin/remood.wad:				rmdtext.exe ${__BUILDROOT}/wad/wadinfo.txt
							@echo [RMDTEX] $(notdir $@)
							@$(call ___RUNCOMMAND,$(___RUNCURDIR)rmdtext.exe ${__BUILDROOT}/wad/wadinfo.txt $@ wad/)

wad:						bin/remood.wad
							-@:

# RMDTEX -- DeuTex Clone For ReMooD
rmdtext.exe:				${__BUILDROOT}/util/rmdtex.c
							@echo [CC] rmdtex
							@$(___HOSTCC) -o $@ $<

# Clean Targets

.PHONY: clean-client
clean-client:				${___CLDEL}
							@echo [RM] $(notdir ${___CLEXENAME})
							@$(call ___RUNCOMMAND,$(___DELETE) $(call ___DOSPATH,bin/${___CLEXENAME})) > $(___DEVNULL)

.PHONY: ccl___o/c/%.${___OBJEXT}
ccl___o/c/%.${___OBJEXT}:	
							@$(if $(wildcard $(subst ccl___,,$@)),echo [RM] $(notdir $(subst ccl___,,$(basename $@)).o),)
							@$(if $(wildcard $(subst ccl___,,$@)),$(call ___RUNCOMMAND,$(___DELETE) $(call ___DOSPATH,$(subst ccl___,,$@))) > $(___DEVNULL),)

ifndef ___NOSERVER

.PHONY: clean-server
clean-server:				${___SVDEL}
							@echo [RM] $(notdir ${___SVEXENAME})
							@$(call ___RUNCOMMAND,$(___DELETE) $(call ___DOSPATH,bin/${___SVEXENAME})) > $(___DEVNULL)

.PHONY: csv___o/c/%.${___OBJEXT}
csv___o/s/%.${___OBJEXT}:	
							@$(if $(wildcard $(subst ccl___,,$@)),echo [RM] $(notdir $(subst ccl___,,$(basename $@)).o),)
							@$(if $(wildcard $(subst ccl___,,$@)),$(call ___RUNCOMMAND,$(___DELETE) $(call ___DOSPATH,$(subst ccl___,,$@))) > $(___DEVNULL),)

endif

## The ReMooD JAR ###

.PHONY: remood.jar
remood.jar:					
							ant -f build.xml jar

