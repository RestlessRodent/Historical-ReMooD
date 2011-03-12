# ########   ###### #####   #####  ######   ######  ######
# ##     ##  ##     ##  ## ##  ## ##    ## ##    ## ##   ##
# ##     ##  ##     ##   ###   ## ##    ## ##    ## ##    ##
# ########   ####   ##    #    ## ##    ## ##    ## ##    ##
# ##    ##   ##     ##         ## ##    ## ##    ## ##    ##
# ##     ##  ##     ##         ## ##    ## ##    ## ##   ##
# ##      ## ###### ##         ##  ######   ######  ######
#                      http://remood.org/
# -----------------------------------------------------------------------------
# Project Leader:    GhostlyDeath           (ghostlydeath@remood.org)
# Project Co-Leader: RedZTag                (jostol@remood.org)
# Members:           TetrisMaster512        (tetrismaster512@hotmail.com)
# -----------------------------------------------------------------------------
# Copyright (C) 2008-2010 The ReMooD Team.
# Copyright (C) 2009-2010 GhostlyDeath (ghostlydeath@remood.org)
# -----------------------------------------------------------------------------
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# -----------------------------------------------------------------------------
# GNU Makefile Reborn! New and Improved, and ready to kick ass!
# Needs to be rewritten again!

#####################################################
### -1ST: DEPRECATED SUPPORT FOR THE OLD MAKEFILE ###
#####################################################
# Some of this dates back to Legacy 1.42!

#### LLVM ####
ifdef LLVM
	REMOODBINPREFIX=$(BINPREFIX)llvm-
else
	ifdef BINPREFIX
		REMOODBINPREFIX=$(BINPREFIX)
	endif
endif

#### BINSUFFIX ####
ifdef BINSUFFIX
	REMOODBINSUFFIX=$(BINSUFFIX)
endif

#### TARGETS ####
ifdef WINDOWS
	REMOODTARGET=win32
else
	ifdef WINDOWS64
		REMOODTARGET=win64
	else
		ifdef LINUX
			REMOODTARGET=linux
		else
			ifdef OSTARGET_LINUX
				REMOODTARGET=linux
			else
				ifdef OSTARGET_WINDOWS
					REMOODTARGET=win32
				else
					ifdef OSTARGET_WINDOWS64
						REMOODTARGET=win64
					else
						ifdef OSDETECT_WINDOWS
							REMOODTARGET=win32
						else
							ifdef OSDETECT_LINUX
								REMOODTARGET=linux
							else
								ifdef OSDETECT_FREEBSD
									REMOODTARGET=freebsd
								else
									ifdef OSDETECT_MSDOS
										REMOODTARGET=dos
									else
										ifdef OSDETECT_MACOS
											REMOODTARGET=macosx
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
endif

#### INTERFACE ####
ifdef SDL
	REMOODINTERFACE=sdl
else
	ifdef SERVER
		REMOODINTERFACE=server
	else
		ifdef SUBSYSTEM_SDL
			REMOODINTERFACE=sdl
		else
			ifdef SUBSYSTEM_OLDSDL
				REMOODINTERFACE=oldsdl
			else
				ifdef SUBSYSTEM_PALMOS
					REMOODINTERFACE=palmos
				else
					REMOODINTERFACE=sdl
				endif
			endif
		endif
	endif
endif

#### DIRECTORIES ####
ifdef TRUNKDIR
	REMOODTRUNK=$(TRUNKDIR)
endif

ifdef OFFSETDIR_SRC
	REMOODINSRCOFFSET=$(OFFSETDIR_SRC)
endif

ifdef OFFSETDIR_RC
	REMOODINRCOFFSET=$(OFFSETDIR_RC)
endif

ifdef OFFSETDIR_WAD
	REMOODINWADOFFSET=$(OFFSETDIR_WAD)
endif

ifdef OFFSETDIR_UTIL
	REMOODINUTILOFFSET=$(OFFSETDIR_UTIL)
endif

ifdef OFFSETDIR_OUT
	REMOODOUTBINOFFSET=$(OFFSETDIR_OUT)
endif

ifdef OFFSETDIR_OUTOBJ
	REMOODOUTOBJOFFSET=$(OFFSETDIR_OUTOBJ)
endif

ifdef OFFSETDIR_OUTWAD
	REMOODOUTWADOFFSET=$(OFFSETDIR_OUTWAD)
endif

ifdef OFFSETDIR_INPUT
	REMOODINSRCOFFSET = $(OFFSETDIR_INPUT)
	REMOODINWADOFFSET = $(OFFSETDIR_INPUT)
endif

ifdef OFFSETDIR_OUTPUT
	REMOODOUTBINOFFSET = $(OFFSETDIR_OUTPUT)
	REMOODOUTOBJOFFSET = $(OFFSETDIR_OUTPUT)
endif

ifdef OFFSETDIR_ALL
	REMOODINSRCOFFSET=$(OFFSETDIR_ALL)
	REMOODINRCOFFSET=$(OFFSETDIR_ALL)
	REMOODINWADOFFSET=$(OFFSETDIR_ALL)
	REMOODINUTILOFFSET=$(OFFSETDIR_ALL)
	REMOODOUTBINOFFSET=$(OFFSETDIR_ALL)
	REMOODOUTOBJOFFSET=$(OFFSETDIR_ALL)
	REMOODOUTWADOFFSET=$(OFFSETDIR_ALL)
endif

##############################
### 0TH: LOCAL TO INTERNAL ###
##############################

ifdef REMOODBINPREFIX
	_INTERNAL_BINPREFIX =$(REMOODBINPREFIX)
endif
ifdef REMOODBINSUFFIX
	_INTERNAL_BINSUFFIX =$(REMOODBINSUFFIX)
endif
ifdef REMOODTARGET
	_INTERNAL_TARGET    =$(REMOODTARGET)
endif
ifdef REMOODINTERFACE
	_INTERNAL_INTERFACE =$(REMOODINTERFACE)
endif
ifdef REMOODTRUNK
	_INTERNAL_TRUNK     =$(REMOODTRUNK)
endif
ifdef REMOODINSRCOFFSET
	_INTERNAL_OFFSET_IN_SRC=$(REMOODINSRCOFFSET)
endif
ifdef REMOODINRCOFFSET
	_INTERNAL_OFFSET_IN_RC=$(REMOODINRCOFFSET)
endif
ifdef REMOODINWADOFFSET
	_INTERNAL_OFFSET_IN_WAD=$(REMOODINWADOFFSET)
endif
ifdef REMOODINUTILOFFSET
	_INTERNAL_OFFSET_IN_UTIL=$(REMOODINUTILOFFSET)
endif
ifdef REMOODOUTBINOFFSET
	_INTERNAL_OFFSET_OUT_BIN=$(REMOODOUTBINOFFSET)
	_INTERNAL_OFFSET_OUT_WADNOMINAL=$(REMOODOUTBINOFFSET)
endif
ifdef REMOODOUTOBJOFFSET
	_INTERNAL_OFFSET_OUT_OBJ=$(REMOODOUTOBJOFFSET)
endif
ifdef REMOODOUTWADOFFSET
	_INTERNAL_OFFSET_OUT_WAD=$(REMOODOUTWADOFFSET)
endif
ifdef REMOODBINDIR
	_INTERNAL_OUTDIR_BIN = $(REMOODBINDIR)
endif
ifdef REMOODOBJDIR
	_INTERNAL_OUTDIR_OBJ = $(REMOODOBJDIR)
endif
ifdef REMOODDEBIANMAINTAINER
	_INTERNAL_DEB_MAINTAINER=$(REMOODDEBIANMAINTAINER)
else
	_INTERNAL_DEB_MAINTAINER=Unknown
endif
ifdef REMOODDEBIANMAINTAINEREMAIL
	_INTERNAL_DEB_MAINTAINEREMAIL=$(REMOODDEBIANMAINTAINEREMAIL)
else
	_INTERNAL_DEB_MAINTAINEREMAIL=unknown@remood.org
endif
ifdef REMOODNOCCPARM
	_INTERNAL_NOCCPARM=$(REMOODNOCCPARM)
	
	# Must be either yes or no...
	ifeq (,$(findstring yes,$(strip $(_INTERNAL_NOCCPARM))))
		ifeq (,$(findstring no,$(strip $(_INTERNAL_NOCCPARM))))
			ifeq (,$(findstring YES,$(strip $(_INTERNAL_NOCCPARM))))
				ifeq (,$(findstring NO,$(strip $(_INTERNAL_NOCCPARM))))
					ifeq (,$(findstring y,$(strip $(_INTERNAL_NOCCPARM))))
						ifeq (,$(findstring n,$(strip $(_INTERNAL_NOCCPARM))))
							_INTERNAL_NOCCPARM=check
						else
							_INTERNAL_NOCCPARM=no
						endif
					else
						_INTERNAL_NOCCPARM=yes
					endif
				else
					_INTERNAL_NOCCPARM=no
				endif
			else
				_INTERNAL_NOCCPARM=yes
			endif
		else
			_INTERNAL_NOCCPARM=no
		endif
	else
		_INTERNAL_NOCCPARM=yes
	endif
endif

ifdef REMOODUSEGCJ
	_INTERNAL_USEGCJ=$(REMOODUSEGCJ)
	
	# Must be either yes or no...
	ifeq (,$(findstring yes,$(strip $(_INTERNAL_USEGCJ))))
		ifeq (,$(findstring no,$(strip $(_INTERNAL_USEGCJ))))
			ifeq (,$(findstring YES,$(strip $(_INTERNAL_USEGCJ))))
				ifeq (,$(findstring NO,$(strip $(_INTERNAL_USEGCJ))))
					ifeq (,$(findstring y,$(strip $(_INTERNAL_USEGCJ))))
						ifeq (,$(findstring n,$(strip $(_INTERNAL_USEGCJ))))
							_INTERNAL_USEGCJ=no
						else
							_INTERNAL_USEGCJ=no
						endif
					else
						_INTERNAL_USEGCJ=yes
					endif
				else
					_INTERNAL_USEGCJ=no
				endif
			else
				_INTERNAL_USEGCJ=yes
			endif
		else
			_INTERNAL_USEGCJ=no
		endif
	else
		_INTERNAL_USEGCJ=yes
	endif
else
	_INTERNAL_USEGCJ=no
endif

###################################################################
### 1ST: DETERMINE THE OPERATING SYSTEM WHICH WE ARE RUNNING ON ###
###################################################################

# Determine the Host Operating system
ifneq (,$(findstring linux,$(strip $(shell uname -s))))
	_INTERNAL_HOST=linux
else
	ifneq (,$(findstring Linux,$(strip $(shell uname -s))))
		_INTERNAL_HOST=linux
	else
		ifneq (,$(findstring FreeBSD,$(strip $(shell uname -s))))
			_INTERNAL_HOST=freebsd
		else
			ifneq (,$(findstring Darwin,$(strip $(shell uname -s))))
				_INTERNAL_HOST=macosx
			else
				ifneq (,$(findstring Windows,$(strip $(shell pr_ver.bat))))
					ifneq (,$(findstring AMD64,$(strip $(PROCESSOR_ARCHITECTURE))))
						_INTERNAL_HOST=win64
					else
						_INTERNAL_HOST=win32
					endif
				else
					ifneq (,$(findstring ReactOS,$(strip $(shell pr_ver.bat))))
						_INTERNAL_HOST=win32
					else
						ifneq (,$(findstring DOS,$(strip $(shell pr_ver.bat))))
							_INTERNAL_HOST=dos
						else
							# WINE
							ifneq (,$(findstring CMD,$(strip $(shell pr_ver.bat))))
								_INTERNAL_HOST=win32
							endif
						endif
					endif
				endif
			endif
		endif
	endif
endif

# Something is wrong with the following statement
ifneq (,$(findstring win32,$(_INTERNAL_HOST))$(findstring win64,$(_INTERNAL_HOST))$(findstring dos,$(_INTERNAL_HOST)))
	_INTERNAL_RMRF = del
	_INTERNAL_MOVE = move
else
	_INTERNAL_RMRF = rm -rf
	_INTERNAL_MOVE = mv
endif

########################################
### 2ND: DETERMINE OUR TARGET SYSTEM ###
########################################

# This is easy
_INTERNAL_TARGET ?= $(_INTERNAL_HOST)

####################################
### 3RD: DETERMINE OUR INTERFACE ###
####################################

# Default to SDL as usual
ifneq (,$(findstring win32,$(_INTERNAL_TARGET))$(findstring win64,$(_INTERNAL_TARGET))$(findstring linux,$(_INTERNAL_TARGET))$(findstring macosx,$(_INTERNAL_TARGET))$(findstring bsd,$(_INTERNAL_TARGET)))
	_INTERNAL_INTERFACE ?= sdl
else
	ifneq (,$(findstring dos,$(_INTERNAL_TARGET)))
		_INTERNAL_INTERFACE ?= univdos
	else
		ifneq (,$(findstring palmos,$(_INTERNAL_TARGET)))
			_INTERNAL_INTERFACE ?= palmos
		else
			ifneq (,$(findstring wince,$(_INTERNAL_TARGET)))
				_INTERNAL_INTERFACE ?= wince
			else
				_INTERNAL_INTERFACE ?= server
			endif
		endif
	endif
endif

#####################################################
### 4TH: DETERMINE OUR COMPILER, OUTPUT, AND MORE ###
#####################################################

## Binaries

# Dev-C++'s Mingw32 has trouble here... and so does mingw32, no cc!
_INTERNAL_NOCCPARM ?= check

ifdef CC
	ifneq (,$(findstring check,$(_INTERNAL_NOCCPARM)))
		# This MAY work on UNIX Land and Windows Land, "Bad command" usually prints to stderr
		# but this also relies on how the commands are executed.
		# must have version since gcc normally prints "gcc: no input files" to stderr
		# if version info is not printed to stdout then this check will fail
		# if it does you must set CC= and REMOODNOCCPARM=no
		
		ifeq (,$(strip $(shell $(_INTERNAL_BINPREFIX)$(CC)$(_INTERNAL_BINSUFFIX) --version)))
			_INTERNAL_NOCCPARM = yes
		else
			# we must check if the command is valid otherwise the invalid -version will be passed to gcc for example!
			ifneq (,$(findstring cc,$(strip $(shell $(_INTERNAL_BINPREFIX)$(CC)$(_INTERNAL_BINSUFFIX) --version))))
				_INTERNAL_NOCCPARM = no
			else
				# If the command was invalid then cc will never be found...
				ifeq (,$(strip $(shell $(_INTERNAL_BINPREFIX)$(CC)$(_INTERNAL_BINSUFFIX) -version)))
					_INTERNAL_NOCCPARM = yes
				else
					# we must check if the command is valid, the common printout is cc...
					ifneq (,$(findstring cc,$(strip $(shell $(_INTERNAL_BINPREFIX)$(CC)$(_INTERNAL_BINSUFFIX) -version))))
						_INTERNAL_NOCCPARM = no
					else
						# If the command was invalid then cc will never be found...
						ifeq (,$(strip $(shell $(_INTERNAL_BINPREFIX)$(CC)$(_INTERNAL_BINSUFFIX) -V)))
							_INTERNAL_NOCCPARM = yes
						else
							_INTERNAL_NOCCPARM = no
						endif
					endif
				endif
			endif
		endif
	endif
else
	_INTERNAL_NOCCPARM = yes
endif

# Set binary based on flag
ifneq (,$(findstring yes,$(_INTERNAL_NOCCPARM)))
	_INTERNAL_CC      = $(_INTERNAL_BINPREFIX)gcc$(_INTERNAL_BINSUFFIX)
	_INTERNAL_LD      = $(_INTERNAL_BINPREFIX)gcc$(_INTERNAL_BINSUFFIX)
else
	_INTERNAL_CC      = $(_INTERNAL_BINPREFIX)$(CC)$(_INTERNAL_BINSUFFIX)
	_INTERNAL_LD      = $(_INTERNAL_BINPREFIX)$(CC)$(_INTERNAL_BINSUFFIX)
endif

_INTERNAL_OBJCOPY = $(_INTERNAL_BINPREFIX)objcopy$(_INTERNAL_BINSUFFIX)
_INTERNAL_OBJDUMP = $(_INTERNAL_BINPREFIX)objdump$(_INTERNAL_BINSUFFIX)
_INTERNAL_STRIP   = $(_INTERNAL_BINPREFIX)strip$(_INTERNAL_BINSUFFIX)
_INTERNAL_WINDRES = $(_INTERNAL_BINPREFIX)windres$(_INTERNAL_BINSUFFIX)

# Java and GCJ Mess
ifneq (,$(findstring no,$(_INTERNAL_USEGCJ)))
	_INTERNAL_JAVAC      = javac
	_INTERNAL_JAVACPFLAG = -cp
	_INTERNAL_JAVAFLAGS  = 
else
	_INTERNAL_JAVAC      = gcj
	_INTERNAL_JAVACPFLAG = --classpath
	_INTERNAL_JAVAFLAGS  = -C -fbootstrap-classes
endif

_INTERNAL_JAVAJAR = jar

# Find DEUTEX
ifneq (,$(findstring DeuTex,$(strip $(shell deutex --version))))
	_INTERNAL_DEUTEX  = deutex
else
	ifneq (,$(findstring DeuSF,$(strip $(shell deutex --version))))
		_INTERNAL_DEUTEX = deusf
	endif
endif

# Binary and object naming scheme
ifdef DEBUG
	ifneq (,$(findstring dos,$(_INTERNAL_TARGET)))
		_INTERNAL_DEBUGEXTENSION = d
	else
		_INTERNAL_DEBUGEXTENSION = -dbg
	endif
	
	_INTERNAL_DEBUGOBJECTBASE = d
endif

ifneq (,$(findstring server,$(_INTERNAL_INTERFACE)))
	_INTERNAL_SERVEREXTENSION = -server
	_INTERNAL_SERVEROBJECTBASE = s
endif

ifneq (,$(findstring win32,$(_INTERNAL_TARGET))$(findstring dos,$(_INTERNAL_TARGET)))
	_INTERNAL_EXESUFFIX    = $(_INTERNAL_SERVEREXTENSION)$(_INTERNAL_DEBUGEXTENSION)
	_INTERNAL_EXEEXTENSION = .exe
else
	ifneq (,$(findstring win64,$(_INTERNAL_TARGET)))
		_INTERNAL_EXESUFFIX    = 64$(_INTERNAL_SERVEREXTENSION)$(_INTERNAL_DEBUGEXTENSION)
		_INTERNAL_EXEEXTENSION = .exe
	else
		ifneq (,$(findstring wince,$(_INTERNAL_TARGET)))
			_INTERNAL_EXESUFFIX    = ce$(_INTERNAL_SERVEREXTENSION)$(_INTERNAL_DEBUGEXTENSION)
			_INTERNAL_EXEEXTENSION = .exe
		else
			ifneq (,$(findstring palmos,$(_INTERNAL_TARGET)))
				_INTERNAL_EXESUFFIX    = $(_INTERNAL_SERVEREXTENSION)$(_INTERNAL_DEBUGEXTENSION)
				_INTERNAL_EXEEXTENSION = .bin
			else
				_INTERNAL_EXESUFFIX    = $(_INTERNAL_SERVEREXTENSION)$(_INTERNAL_DEBUGEXTENSION)
			endif
		endif
	endif
endif

ifneq (,$(findstring win32,$(_INTERNAL_TARGET)))
	_INTERNAL_OBJECTBASE = w3
else
	ifneq (,$(findstring win64,$(_INTERNAL_TARGET)))
		_INTERNAL_OBJECTBASE = w6
	else
		ifneq (,$(findstring linux,$(_INTERNAL_TARGET)))
			_INTERNAL_OBJECTBASE = l
		else
			ifneq (,$(findstring freebsd,$(_INTERNAL_TARGET)))
				_INTERNAL_OBJECTBASE = f
			else
				ifneq (,$(findstring dos,$(_INTERNAL_TARGET)))
					_INTERNAL_OBJECTBASE = 
				else
					ifneq (,$(findstring macosx,$(_INTERNAL_TARGET)))
						_INTERNAL_OBJECTBASE = m
					else
						ifneq (,$(findstring palmos,$(_INTERNAL_TARGET)))
							_INTERNAL_OBJECTBASE = p
						else
							ifneq (,$(findstring wince,$(_INTERNAL_TARGET)))
								_INTERNAL_OBJECTBASE = c
							endif
						endif
					endif
				endif
			endif
		endif
	endif
endif

# Headless?
ifneq (,$(findstring headless,$(_INTERNAL_INTERFACE)))
	_INTERNAL_INTERFACEDIR=server
else
	_INTERNAL_INTERFACEDIR=$(_INTERNAL_INTERFACE)
endif

# Directories
_INTERNAL_TRUNK          ?= .
_INTERNAL_OFFSET_IN_DOC  ?= $(_INTERNAL_TRUNK)
_INTERNAL_OFFSET_IN_SRC  ?= $(_INTERNAL_TRUNK)
_INTERNAL_OFFSET_IN_RC   ?= $(_INTERNAL_TRUNK)
_INTERNAL_OFFSET_IN_WAD  ?= $(_INTERNAL_TRUNK)
_INTERNAL_OFFSET_IN_UTIL ?= $(_INTERNAL_TRUNK)
_INTERNAL_OFFSET_OUT_BIN ?= $(_INTERNAL_TRUNK)
_INTERNAL_OFFSET_OUT_OBJ ?= $(_INTERNAL_TRUNK)
_INTERNAL_OFFSET_OUT_WAD ?= $(_INTERNAL_TRUNK)
_INTERNAL_OFFSET_OUT_WADNOMINAL ?= $(_INTERNAL_TRUNK)

### For make... ###
ifeq (,$(_INTERNAL_OUTDIR_BIN))
	_INTERNAL_OUTDIR_BIN ?= $(_INTERNAL_OFFSET_OUT_BIN)/bin
endif

ifeq (,$(_INTERNAL_OUTDIR_OBJ))
	_INTERNAL_OUTDIR_OBJ ?= $(_INTERNAL_OFFSET_OUT_OBJ)/objs
endif

ifeq (,$(_INTERNAL_OUTDIR_WAD))
	_INTERNAL_OUTDIR_WAD ?= $(_INTERNAL_OFFSET_OUT_WAD)/bin
endif

ifeq (,$(_INTERNAL_OUTDIR_WADNOMINAL))
	_INTERNAL_OUTDIR_WADNOMINAL ?= $(_INTERNAL_OFFSET_OUT_WADNOMINAL)/bin
endif

_INTERNAL_IN_DOC      = $(_INTERNAL_OFFSET_IN_DOC)/doc
_INTERNAL_IN_SRC      = $(_INTERNAL_OFFSET_IN_SRC)/src
_INTERNAL_IN_WAD      = $(_INTERNAL_OFFSET_IN_WAD)/wad
_INTERNAL_IN_RC       = $(_INTERNAL_OFFSET_IN_RC)/rc
_INTERNAL_IN_UTIL     = $(_INTERNAL_OFFSET_IN_UTIL)/util
_INTERNAL_IN_UTILLNCH = $(_INTERNAL_OFFSET_IN_UTIL)/util/launcher
_INTERNAL_IN_JAVALNCH = $(_INTERNAL_OFFSET_IN_UTIL)/util/javalnch
_INTERNAL_IN_DCDLL    = $(_INTERNAL_OFFSET_IN_UTIL)/util/dcdll
_INTERNAL_IN_ISRC     = $(_INTERNAL_IN_SRC)/$(_INTERNAL_INTERFACEDIR)
_INTERNAL_OUT_EXE     = $(_INTERNAL_OUTDIR_BIN)/$(_INTERNAL_EXEPREFIX)remood$(_INTERNAL_EXESUFFIX)$(_INTERNAL_EXEEXTENSION)
_INTERNAL_OUT_EXELNCH = $(_INTERNAL_OUTDIR_BIN)/$(_INTERNAL_EXEPREFIX)remood-launcher$(_INTERNAL_EXESUFFIX)$(_INTERNAL_EXEEXTENSION)
_INTERNAL_OUT_DCDLL   = $(_INTERNAL_OUTDIR_BIN)/remood.dll
_INTERNAL_OUT_PALMPRC = $(_INTERNAL_OUTDIR_BIN)/remood$(_INTERNAL_EXESUFFIX).prc
_INTERNAL_OUT_WINCAB  = $(_INTERNAL_OUTDIR_BIN)/remood$(_INTERNAL_EXESUFFIX).cab
_INTERNAL_OUT_JARLNCH = $(_INTERNAL_OUTDIR_BIN)/remoodlauncher.jar
_INTERNAL_OUT_OBJROOT = $(_INTERNAL_OUTDIR_OBJ)
_INTERNAL_OUT_OBJ     = $(_INTERNAL_OUTDIR_OBJ)/$(_INTERNAL_OBJECTBASE)$(_INTERNAL_SERVEROBJECTBASE)$(_INTERNAL_DEBUGOBJECTBASE)
_INTERNAL_OUT_WAD     = $(_INTERNAL_OUTDIR_WAD)/remood.wad
_INTERNAL_OUT_WADNOMIMAL = $(_INTERNAL_OUTDIR_WADNOMINAL)/remood.wad
_INTERNAL_OUT_WADCOMPLETE = ../$(_INTERNAL_OUT_WAD)

_INTERNAL_OUT_EXESTRIPPED     = $(_INTERNAL_EXEPREFIX)remood$(_INTERNAL_EXESUFFIX)$(_INTERNAL_EXEEXTENSION)
_INTERNAL_OUT_EXELNCHSTRIPPED = $(_INTERNAL_EXEPREFIX)remood-launcher$(_INTERNAL_EXESUFFIX)$(_INTERNAL_EXEEXTENSION)

##########################################
### 5TH: PLATFORM SPECIFIC BUILD FLAGS ###
##########################################

ifneq (,$(findstring win32,$(_INTERNAL_TARGET)))
	_INTERNAL_SCC_FLAGS = -DWIN32 -D_WIN32 -DENABLEMULTITHREADING -D_REMOOD_TARGET_WIN32
	_INTERNAL_SLD_FLAGS = -mwindows -ldxguid -ldinput -ldsound -lddraw -lwinmm -lwsock32 -lkernel32 -luser32 -lgdi32 -lwinspool -lcomdlg32 -ladvapi32 -lshell32 -lole32 -loleaut32 -luuid -lodbc32 -lodbccp32
	
	_INTERNAL_SLAUNCHERCC_FLAGS = -D_WIN32
	_INTERNAL_SLAUNCHERLD_FLAGS = -mwindows -lwsock32 -lkernel32 -luser32 -lgdi32 -lcomdlg32 -ladvapi32 -lshell32 -lcomctl32
else
	ifneq (,$(findstring win64,$(_INTERNAL_TARGET)))
		_INTERNAL_SCC_FLAGS = -DWIN64 -D_WIN64 -DWIN32 -D_WIN32 -DENABLEMULTITHREADING -D_REMOOD_TARGET_WIN64
		_INTERNAL_SLD_FLAGS = -mwindows -ldxguid -ldinput -ldsound -lddraw -lwinmm -lws2_32 -lkernel32 -luser32 -lgdi32 -lwinspool -lcomdlg32 -ladvapi32 -lshell32 -lole32 -loleaut32 -luuid -lodbc32 -lodbccp32
		
		_INTERNAL_SLAUNCHERCC_FLAGS = -D_WIN32 -D_WIN64
		_INTERNAL_SLAUNCHERLD_FLAGS = -mwindows -lws2_32 -lkernel32 -luser32 -lgdi32 -lcomdlg32 -ladvapi32 -lshell32 -lcomctl32
	else
		ifneq (,$(findstring dos,$(_INTERNAL_TARGET)))
			_INTERNAL_SCC_FLAGS = -D__MSDOS__ -D_REMOOD_TARGET_DOS
			_INTERNAL_SLD_FLAGS =
		else
			ifneq (,$(findstring linux,$(_INTERNAL_TARGET)))
				_INTERNAL_SCC_FLAGS = -DLINUX -DENABLEMULTITHREADING -D_REMOOD_TARGET_LINUX
				_INTERNAL_SLD_FLAGS = -pthread
			else
				ifneq (,$(findstring freebsd,$(_INTERNAL_TARGET)))
					_INTERNAL_SCC_FLAGS = -DFREEBSD -DENABLEMULTITHREADING -D_REMOOD_TARGET_FREEBSD
					_INTERNAL_SLD_FLAGS = -pthread
				else
					ifneq (,$(findstring macosx,$(_INTERNAL_TARGET)))
						_INTERNAL_SCC_FLAGS = -D_REMOOD_TARGET_MACOSX
						_INTERNAL_SLD_FLAGS = 
					else
						ifneq (,$(findstring palmos,$(_INTERNAL_TARGET)))
							_INTERNAL_SCC_FLAGS = -D_REMOOD_TARGET_PALMOS -D_REMOOD_DISABLECLIB -fPIC -ffixed-r9 -Wcast-align
							_INTERNAL_SLD_FLAGS = -nostdlib -msoft-float -lgcc -fPIC -ffixed-r9
						else
							ifneq (,$(findstring wince,$(_INTERNAL_TARGET)))
								_INTERNAL_SCC_FLAGS = -D_REMOOD_TARGET_WINCE -D_WIN32_WCE -D_WIN32 -Wcast-align
								_INTERNAL_SLD_FLAGS = -fPIC
								
								ifeq (,$(REMOODNOSOFTFLOAT))
									_INTERNAL_SCC_FLAGS := $(_INTERNAL_SCC_FLAGS) -msoft-float 
									_INTERNAL_SLD_FLAGS := $(_INTERNAL_SLD_FLAGS) -msoft-float 
								endif
							endif
						endif
					endif
				endif
			endif
		endif
	endif
endif

###########################################
### 6TH: INTERFACE SPECIFIC BUILD FLAGS ###
###########################################

			### SDL ###
ifneq (,$(findstring sdl,$(_INTERNAL_INTERFACE)))
	ifneq (,$(findstring win32,$(_INTERNAL_TARGET))$(findstring win64,$(_INTERNAL_TARGET)))
		ifdef SDL_INCLUDE
			_INTERNAL_ICC_FLAGS = $(SDL_INCLUDE)
		else
			_INTERNAL_ICC_FLAGS = -I. -ISDL
		endif
		
		ifdef SDL_LIB
			_INTERNAL_ILD_FLAGS = $(SDL_LIB) -lSDL.dll -lm
		else
			_INTERNAL_ILD_FLAGS = -L. -LSDL -lSDL.dll -lm
		endif
	else
		ifdef SDL_INCLUDE
			_INTERNAL_ICC_FLAGS = $(SDL_INCLUDE)
		else
			_INTERNAL_ICC_FLAGS = $(shell sdl-config --cflags) -DDIRECTFULLSCREEN -DSDL -D_INTERFACE_SDL
		endif
		
		ifdef SDL_LIB
			_INTERNAL_ILD_FLAGS = $(SDL_LIB) -lSDL -lm
		else
			_INTERNAL_ILD_FLAGS = $(shell sdl-config --libs) -DDIRECTFULLSCREEN -DSDL  -lm
		endif
	endif
else
			### Old SDL ###
	ifneq (,$(findstring oldsdl,$(_INTERNAL_INTERFACE)))
		ifneq (,$(findstring win32,$(_INTERNAL_TARGET))$(findstring win64,$(_INTERNAL_TARGET)))
			ifdef SDL_INCLUDE
				_INTERNAL_ICC_FLAGS = $(SDL_INCLUDE)
			else
				_INTERNAL_ICC_FLAGS = -I. -ISDL
			endif
		
			ifdef SDL_LIB
				_INTERNAL_ILD_FLAGS = $(SDL_LIB) -lSDL.dll
			else
				_INTERNAL_ILD_FLAGS = -L. -LSDL -lSDL.dll
			endif
		else
			ifdef SDL_INCLUDE
				_INTERNAL_ICC_FLAGS = $(SDL_INCLUDE)
			else
				_INTERNAL_ICC_FLAGS = $(shell sdl-config --cflags) -DDIRECTFULLSCREEN -DSDL -D_INTERFACE_SDL
			endif
		
			ifdef SDL_LIB
				_INTERNAL_ILD_FLAGS = $(SDL_LIB) -lSDL
			else
				_INTERNAL_ILD_FLAGS = $(shell sdl-config --libs) -DDIRECTFULLSCREEN -DSDL
			endif
		endif
	else
			### Server ###
		ifneq (,$(findstring server,$(_INTERNAL_INTERFACE))$(findstring headless,$(_INTERNAL_INTERFACE)))
			_INTERNAL_ICC_FLAGS = -D_INTERFACE_SERVER
			_INTERNAL_ILD_FLAGS = -lm
		
		else
		
			### UNIVERSAL DOS ###
			ifneq (,$(findstring univdos,$(_INTERNAL_INTERFACE)))
				_INTERNAL_ICC_FLAGS = -D_INTERFACE_UNIVDOS
				_INTERNAL_ILD_FLAGS =
			
			else
			
				### PALM OS ###
				ifneq (,$(findstring palmos,$(_INTERNAL_INTERFACE)))
					_INTERNAL_ICC_FLAGS = -D_INTERFACE_PALMOS -D_REMOOD_DISABLECLIB -fno-builtin
					_INTERNAL_ILD_FLAGS =
				else
					
					### WINDOWS CE ###
					ifneq (,$(findstring wince,$(_INTERNAL_INTERFACE)))
						#-mno-thumb-interwork -mno-sched-prolog
						_INTERNAL_ICC_FLAGS = -D_INTERFACE_WINCE
						_INTERNAL_ILD_FLAGS =
						
						ifeq (,$(REMOODNOSOFTFLOAT))
							_INTERNAL_SCC_FLAGS := $(_INTERNAL_SCC_FLAGS) -msoft-float 
							_INTERNAL_SLD_FLAGS := $(_INTERNAL_SLD_FLAGS) -msoft-float 
						endif
					endif
				endif
			endif
		endif
	endif
endif

##########################
### 7TH: BUILD TARGETS ###
##########################

ifneq (,$(findstring server,$(_INTERNAL_INTERFACE)))
	_INTERNAL_SRVCC_FLAGS = -DGAMESERVER -UGAMECLIENT
	_INTERNAL_SRVRC_FLAGS = -DGAMESERVER -UGAMECLIENT
else
	_INTERNAL_SRVCC_FLAGS = -DGAMECLIENT -UGAMESERVER
	_INTERNAL_SRVRC_FLAGS = -DGAMECLIENT -UGAMESERVER
endif

ifdef DEBUG
	ifneq (,$(findstring palmos,$(_INTERNAL_TARGET))$(findstring wince,$(_INTERNAL_TARGET)))
		_INTERNAL_DEBUGLEVEL=-g1 -O0
	else
		_INTERNAL_DEBUGLEVEL=-g3 -O0
	endif
	
	_INTERNAL_CC_FLAGS = $(_INTERNAL_SRVCC_FLAGS) -D_DEBUG $(_INTERNAL_DEBUGLEVEL) $(_INTERNAL_ICC_FLAGS) $(_INTERNAL_SCC_FLAGS) $(CFLAGS) -I$(_INTERNAL_IN_ISRC) -I$(_INTERNAL_IN_SRC) -I. -Wall
	_INTERNAL_LD_FLAGS = $(_INTERNAL_ILD_FLAGS) $(_INTERNAL_SLD_FLAGS) $(LDFLAGS) $(_INTERNAL_DEBUGLEVEL)
	_INTERNAL_RC_FLAGS = $(_INTERNAL_SRVRC_FLAGS) -D_DEBUG
	_INTERNAL_LAUNCHERCC_FLAGS = $(_INTERNAL_SLAUNCHERCC_FLAGS) -D_DEBUG $(_INTERNAL_DEBUGLEVEL) -Wall
	_INTERNAL_LAUNCHERLD_FLAGS = $(_INTERNAL_SLAUNCHERLD_FLAGS) $(_INTERNAL_DEBUGLEVEL)
else
	ifeq (,$(findstring palmos,$(_INTERNAL_TARGET))$(findstring wince,$(_INTERNAL_TARGET)))
		_INTERNAL_GCC_OPTIMIZE = -g0 -O1 -fno-strict-aliasing
	else
		_INTERNAL_GCC_OPTIMIZE = -g0 -Os
	endif
	
	_INTERNAL_CC_FLAGS = $(_INTERNAL_SRVCC_FLAGS) -DNDEBUG $(_INTERNAL_ICC_FLAGS) $(_INTERNAL_SCC_FLAGS) $(CFLAGS) -I$(_INTERNAL_IN_ISRC) -I$(_INTERNAL_IN_SRC) -I. -Wall $(_INTERNAL_GCC_OPTIMIZE)
	_INTERNAL_LD_FLAGS = $(_INTERNAL_ILD_FLAGS) $(_INTERNAL_SLD_FLAGS) $(LDFLAGS) $(_INTERNAL_GCC_OPTIMIZE)
	_INTERNAL_RC_FLAGS = $(_INTERNAL_SRVRC_FLAGS) -DNDEBUG
	_INTERNAL_LAUNCHERCC_FLAGS = $(_INTERNAL_SLAUNCHERCC_FLAGS) -DNDEBUG $(_INTERNAL_GCC_OPTIMIZE) -Wall
	_INTERNAL_LAUNCHERLD_FLAGS = $(_INTERNAL_SLAUNCHERLD_FLAGS) $(_INTERNAL_GCC_OPTIMIZE)
endif

#### OBJECTS ####
ifneq (,$(findstring win32,$(_INTERNAL_TARGET))$(findstring win64,$(_INTERNAL_TARGET))$(findstring wince,$(_INTERNAL_TARGET)))
	_INTERNAL_REMOODSPECIALOBJECTS=$(_INTERNAL_OUT_OBJ)w32rrc.o
else
	_INTERNAL_REMOODSPECIALOBJECTS=
endif

_INTERNAL_LAUNCHER_OBJECTS = \
	$(_INTERNAL_OUT_OBJ)lnc_w32_lnch.o $(_INTERNAL_OUT_OBJ)w32lrc.o

_INTERNAL_JAVALAUNCHER_OBJECTS = \
	$(_INTERNAL_OUT_OBJROOT)/ReMooDMain.class

_INTERNAL_CONNECTORDLL_OBJECTS = \
	$(_INTERNAL_OUT_OBJ)dll_dllmain.o  $(_INTERNAL_OUT_OBJ)dll_int_conn.o $(_INTERNAL_OUT_OBJ)dll_int_game.o \
	$(_INETRNAL_OUT_OBJ)w32drc.o

_INTERNAL_REMOOD_OBJECTS = \
	$(_INTERNAL_REMOODSPECIALOBJECTS) \
	$(_INTERNAL_OUT_OBJ)i_main.o \
	$(_INTERNAL_OUT_OBJ)i_sound.o \
	$(_INTERNAL_OUT_OBJ)i_system.o \
	$(_INTERNAL_OUT_OBJ)i_net.o \
	$(_INTERNAL_OUT_OBJ)i_thread.o \
	$(_INTERNAL_OUT_OBJ)i_video.o \
	$(_INTERNAL_OUT_OBJ)i_cdmus.o \
	$(_INTERNAL_OUT_OBJ)i_music.o \
	$(_INTERNAL_OUT_OBJ)endtxt.o \
	$(foreach source, $(wildcard $(_INTERNAL_IN_SRC)/*.c), $(_INTERNAL_OUT_OBJ)$(basename $(notdir $(source))).o)

# This could probably be simplified by using eval and foreach (as in foreach eval)
# TODO: Do that!
ifneq (,$(findstring win32,$(_INTERNAL_HOST))$(findstring win64,$(_INTERNAL_HOST))$(findstring dos,$(_INTERNAL_HOST))$(findstring wince,$(_INTERNAL_HOST)))
	_INTERNAL_REMOOD_RMEXE=$(foreach x,$(_INTERNAL_OUT_EXE),$(subst /,\,$(x)))
	_INTERNAL_REMOOD_RMOBJECTS=$(foreach x,$(_INTERNAL_REMOOD_OBJECTS),$(subst /,\,$(x)))
	_INTERNAL_OUT_RMEXELNCH=$(foreach x,$(_INTERNAL_OUT_EXELNCH),$(subst /,\,$(x)))
	_INTERNAL_LAUNCHER_RMOBJECTS=$(foreach x,$(_INTERNAL_LAUNCHER_OBJECTS),$(subst /,\,$(x)))
	_INTERNAL_CONNECTORDLL_RMOBJECTS=$(foreach x,$(_INTERNAL_CONNECTORDLL_OBJECTS),$(subst /,\,$(x)))
	_INTERNAL_OUT_RMDCDLL=$(foreach x,$(_INTERNAL_OUT_DCDLL),$(subst /,\,$(x)))
else
	_INTERNAL_REMOOD_RMEXE=$(_INTERNAL_OUT_EXE)
	_INTERNAL_REMOOD_RMOBJECTS=$(_INTERNAL_REMOOD_OBJECTS)
	_INTERNAL_OUT_RMEXELNCH=$(_INTERNAL_OUT_EXELNCH)
	_INTERNAL_LAUNCHER_RMOBJECTS=$(_INTERNAL_LAUNCHER_OBJECTS)
	_INTERNAL_CONNECTORDLL_RMOBJECTS=$(_INTERNAL_CONNECTORDLL_OBJECTS)
	_INTERNAL_OUT_RMDCDLL=$(_INTERNAL_OUT_DCDLL)
endif

#### TRUE TARGETS ###

## Binary ##
$(_INTERNAL_OUT_EXE):			$(_INTERNAL_REMOOD_OBJECTS)
								@echo %%%%%% Linking $@ %%%%%%
								$(_INTERNAL_CC) -o $@ $(_INTERNAL_REMOOD_OBJECTS) $(_INTERNAL_LD_FLAGS)

## Palm OS PRC ##
$(_INTERNAL_OUT_PALMPRC):		$(_INTERNAL_OUT_EXE)
								

## Windows CE Cab
.PHONY: $(_INTERNAL_OUT_WINCAB)
$(_INTERNAL_OUT_WINCAB):		$(_INTERNAL_OUT_EXE)
								echo "$(_INTERNAL_OUT_EXE) %CE1%/ReMooD" > rmdcabce.txt
								echo "$(_INTERNAL_OUT_WADNOMIMAL) %CE1%\\ReMooD" >> rmdcabce.txt
								echo "$(_INTERNAL_IN_DOC)/readme.htm %CE1%\\ReMooD" >> rmdcabce.txt
								echo "$(_INTERNAL_IN_DOC)/editing.htm %CE1%\\ReMooD" >> rmdcabce.txt
								echo "$(_INTERNAL_IN_DOC)/scripts.htm %CE1%\\ReMooD" >> rmdcabce.txt
								echo "$(_INTERNAL_IN_DOC)/protocol.htm %CE1%\\ReMooD" >> rmdcabce.txt
								echo "$(_INTERNAL_TRUNK)/sys/ReMooD.lnk %CE11%" >> rmdcabce.txt
								pocketpc-cab -p "ReMooD" -a ReMooD rmdcabce.txt $@

## Win32 Launcher ##
$(_INTERNAL_OUT_EXELNCH):		$(_INTERNAL_LAUNCHER_OBJECTS)
								@echo %%%%%% Linking $@ %%%%%%
								$(_INTERNAL_CC) -o $@ $(_INTERNAL_LAUNCHER_OBJECTS) $(_INTERNAL_LAUNCHERLD_FLAGS)

## Java Launcher ##
$(_INTERNAL_OUT_JARLNCH):		clean-java-launcher $(_INTERNAL_JAVALAUNCHER_OBJECTS)
								$(_INTERNAL_RMRF) ReMooD*.class
								$(_INTERNAL_MOVE) $(_INTERNAL_OUT_OBJROOT)/ReMooD*.class ./
								
								$(_INTERNAL_JAVAJAR) vcfm $(_INTERNAL_OUT_JARLNCH) $(_INTERNAL_IN_JAVALNCH)/Manifest.txt -C $(_INTERNAL_IN_JAVALNCH) remood16.gif -C $(_INTERNAL_IN_JAVALNCH) remood24.gif -C $(_INTERNAL_IN_JAVALNCH) remood32.gif -C $(_INTERNAL_IN_JAVALNCH) remood48.gif -C $(_INTERNAL_IN_JAVALNCH) credit.gif ReMooD*.class
								
								$(_INTERNAL_MOVE) ./ReMooD*.class $(_INTERNAL_OUT_OBJROOT)


## Doom Connector DLL ##
$(_INTERNAL_OUT_DCDLL):			$(_INTERNAL_CONNECTORDLL_OBJECTS)
								@echo %%%%%% Linking $@ %%%%%%
								$(_INTERNAL_CC) -o $@ $(_INTERNAL_CONNECTORDLL_OBJECTS) $(_INTERNAL_DLLLD_FLAGS)

## REMOOD.WAD ##
$(_INTERNAL_OUT_WADNOMIMAL):	
								@echo %%%%%% Creating $@ %%%%%%
								(cd $(_INTERNAL_IN_WAD); $(_INTERNAL_DEUTEX) -rgb 0 255 255 -doom2 bootstrap -build wadinfo.txt $(_INTERNAL_OUT_WADCOMPLETE))

## Aliases for targets ##
.DEFAULT: all
.PHONY: all
all:							release launcher dcdll
								

.PHONY: release
release:						$(_INTERNAL_OUT_EXE)
								

.PHONY: launcher
launcher:						$(_INTERNAL_OUT_EXELNCH)
								

.PHONY: java-launcher
java-launcher:					$(_INTERNAL_OUT_JARLNCH)
								

.PHONY: dcdll
dcdll:							$(_INTERNAL_OUT_DCDLL)
								

.PHONY: wad
wad:							clean-wad $(_INTERNAL_OUT_WADNOMIMAL)
								

.PHONY: wince-cab
wince-cab:						$(_INTERNAL_OUT_WINCAB)
								

.PHONY: palm-prc
palm-prc:						$(_INTERNAL_OUT_PALMPRC)
								

## Clean ##
.PHONY: clean
clean:							clean-release clean-launcher clean-dcdll clean-java-launcher clean-palm-prc clean-wince-cab

.PHONY: clean-release
clean-release:					
								$(_INTERNAL_RMRF) $(_INTERNAL_REMOOD_RMOBJECTS) $(_INTERNAL_REMOOD_RMEXE)

.PHONY: clean-launcher
clean-launcher:					
								$(_INTERNAL_RMRF) $(_INTERNAL_LAUNCHER_RMOBJECTS) $(_INTERNAL_OUT_RMEXELNCH)

.PHONY: clean-dcdll
clean-dcdll:					
								$(_INTERNAL_RMRF) $(_INTERNAL_CONNECTORDLL_RMOBJECTS) $(_INTERNAL_OUT_RMDCDLL)

.PHONY: clean-wad
clean-wad:						
								$(_INTERNAL_RMRF) $(_INTERNAL_OUT_WADNOMIMAL)

.PHONY: clean-java-launcher
clean-java-launcher:			
								$(_INTERNAL_RMRF) $(_INTERNAL_JAVALAUNCHER_OBJECTS) $(_INTERNAL_OUT_JARLNCH) $(_INTERNAL_OUT_OBJROOT)/ReMooD*.class

.PHONY: clean-palm-prc
clean-palm-prc:					
								$(_INTERNAL_RMRF) $(_INTERNAL_OUT_PALMPRC)

.PHONY: clean-wince-cab
clean-wince-cab:				
								$(_INTERNAL_RMRF) $(_INTERNAL_OUT_WINCAB)

################################## NON-WINDOWS #################################
.PHONY: install
install:						release wad
								@echo "%%%%%% Making UNIX Tree if it does not already exist (/usr/local) %%%%%%"
								@if (test \! -d /usr); then mkdir /usr; echo "Made /usr"; fi
								@if (test \! -d /usr/local); then mkdir /usr/local; echo "Made /usr/local"; fi
								@if (test \! -d /usr/local/games); then mkdir /usr/local/games; echo "Made /usr/local/games"; fi
								@if (test \! -d /usr/local/share); then mkdir /usr/local/share; echo "Made /usr/local/share"; fi
								@if (test \! -d /usr/local/share/games); then mkdir /usr/local/share/games; echo "Made /usr/local/share/games"; fi
								@if (test \! -d /usr/local/share/games/doom); then mkdir /usr/local/share/games/doom; echo "Made /usr/local/share/games/doom"; fi
								@if (test \! -d /usr/local/share/doc); then mkdir /usr/local/share/doc; echo "Made /usr/local/share/doc"; fi
								@if (test \! -d /usr/local/share/doc/remood); then mkdir /usr/local/share/doc/remood; echo "Made /usr/local/share/doc/remood"; fi
								@if (test \! -d /usr/local/share/applications); then mkdir /usr/local/share/applications; echo "Made /usr/local/share/applications"; fi
								@if (test \! -d /usr/local/share/menu); then mkdir /usr/local/share/menu; echo "Made /usr/local/share/menu"; fi
								@if (test \! -d /usr/local/share/pixmaps); then mkdir /usr/local/share/pixmaps; echo "Made /usr/local/share/pixmaps"; fi
								
								@echo "%%%%%% Copying files %%%%%%"
								cp $(_INTERNAL_OUT_EXE) /usr/local/games/remood.real
								cp $(_INTERNAL_TRUNK)/sys/wrap.sh /usr/local/games/remood
								cp $(_INTERNAL_OUT_WADNOMIMAL) /usr/local/share/games/doom/remood.wad
								cp $(_INTERNAL_IN_DOC)/readme.htm /usr/local/share/doc/remood/readme.htm
								cp $(_INTERNAL_IN_DOC)/editing.htm /usr/local/share/doc/remood/editing.htm
								cp $(_INTERNAL_IN_DOC)/scripts.htm /usr/local/share/doc/remood/scripts.htm
								cp $(_INTERNAL_IN_DOC)/protocol.htm /usr/local/share/doc/remood/protocol.htm
								cp $(_INTERNAL_TRUNK)/sys/remood.desktop /usr/local/share/applications/remood.desktop
								cp $(_INTERNAL_TRUNK)/sys/remood.xpm /usr/local/share/pixmaps/remood.xpm
								cp $(_INTERNAL_TRUNK)/sys/xmenu /usr/local/share/menu/remood
								
								chmod +x /usr/local/games/remood

.PHONY: uninstall
uninstall:						
								@echo "%%%%%% Deleting files from UNIX Tree (/usr/local) %%%%%%"
								rm -f /usr/local/games/remood.real
								rm -f /usr/local/games/remood
								rm -f /usr/local/share/games/doom/remood.wad
								rm -f /usr/local/share/doc/remood/readme.htm
								rm -f /usr/local/share/doc/remood/editing.htm
								rm -f /usr/local/share/doc/remood/scripts.htm
								rm -f /usr/local/share/doc/remood/protocol.htm
								rm -f /usr/local/share/applications/remood.desktop
								rm -f /usr/local/share/pixmaps/remood.xpm
								rm -f /usr/local/share/menu/remood

.PHONY: debian-base
debian-base:					
								if test -d debian_helper; then $(_INTERNAL_RMRF) debian_helper; fi
								
								@echo %%%% Generating Helper Information %%%%
								mkdir debian_helper
								expr $(shell stat -c %s "$(_INTERNAL_OUT_WADNOMIMAL)") + $(shell stat -c %s "$(_INTERNAL_OUT_EXE)") + $(shell stat -c %s "$(_INTERNAL_IN_DOC)/readme.htm") + $(shell stat -c %s "$(_INTERNAL_IN_DOC)/editing.htm") + $(shell stat -c %s "$(_INTERNAL_IN_DOC)/scripts.htm") + $(shell stat -c %s "$(_INTERNAL_IN_DOC)/protocol.htm") + $(shell stat -c %s "$(_INTERNAL_TRUNK)/sys/remood.desktop") + $(shell stat -c %s "$(_INTERNAL_TRUNK)/sys/remood.xpm") + $(shell stat -c %s "$(_INTERNAL_TRUNK)/sys/xmenu") > debian_helper/bytes			
								expr `cat debian_helper/bytes` / 1024 > debian_helper/kibibytes
								cat $(_INTERNAL_TRUNK)/version > debian_helper/version
								(tr -d . < $(_INTERNAL_TRUNK)/version) > debian_helper/strippedversion
								echo `dpkg -l libc6 | tail -n 1` | awk '{ print$$3 }' > debian_helper/libc6ver
								echo `dpkg -l libsdl1.2debian | tail -n 1` | awk '{ print$$3 }' > debian_helper/libsdlver
								$(_INTERNAL_CC) -dumpmachine > debian_helper/gcctarget
								(echo `cat debian_helper/gcctarget` | cut -d - -f1) > debian_helper/architecturebase
								if (expr "`cat debian_helper/architecturebase`" = "" > /dev/null); then (dpkg --print-architecture > debian_helper/architecturebase); fi
								cat debian_helper/architecturebase > debian_helper/architecture
								(if (expr "`cat debian_helper/architecturebase`" = "x86_64" > /dev/null); then (echo "amd64" > debian_helper/architecture); fi)
								(if (expr "`cat debian_helper/architecturebase`" = "i686" > /dev/null); then (echo "i386" > debian_helper/architecture); fi)
								(if (expr "`cat debian_helper/architecturebase`" = "i586" > /dev/null); then (echo "i386" > debian_helper/architecture); fi)
								(if (expr "`cat debian_helper/architecturebase`" = "i486" > /dev/null); then (echo "i386" > debian_helper/architecture); fi)
								
								echo "$(_INTERNAL_DEB_MAINTAINER) ($(_INTERNAL_DEB_MAINTAINEREMAIL))" > debian_helper/maintainer

.PHONY: debian
debian:							wad release debian-base
								@echo %%%%%% Building Debian Package %%%%%%
								
								@echo %%%% Nuking debian/ %%%%
								if test -d debian; then $(_INTERNAL_RMRF) debian; fi
								
								@echo %%%% Creating debian/ %%%%
								mkdir debian/
								mkdir debian/DEBIAN
								mkdir debian/usr
								mkdir debian/usr/share
								mkdir debian/usr/share/applications
								mkdir debian/usr/share/pixmaps
								mkdir debian/usr/share/games
								mkdir debian/usr/share/games/doom
								mkdir debian/usr/share/menu
								mkdir debian/usr/share/doc
								mkdir debian/usr/share/doc/remood
								mkdir debian/usr/games
								
								@echo %%%% Copy generated and pre-generated files %%%%
								
								cp $(_INTERNAL_TRUNK)/sys/debpostinst debian/DEBIAN/postinst
								cp $(_INTERNAL_TRUNK)/sys/debprerm debian/DEBIAN/prerm
								cp $(_INTERNAL_OUT_EXE) debian/usr/games
								mv debian/usr/games/$(_INTERNAL_OUT_EXESTRIPPED) debian/usr/games/remood.real
								cp $(_INTERNAL_TRUNK)/sys/wrap.sh debian/usr/games/remood
								cp $(_INTERNAL_OUT_WADNOMIMAL) debian/usr/share/games/doom
								cp $(_INTERNAL_IN_DOC)/readme.htm debian/usr/share/doc/remood
								cp $(_INTERNAL_IN_DOC)/editing.htm debian/usr/share/doc/remood
								cp $(_INTERNAL_IN_DOC)/scripts.htm debian/usr/share/doc/remood
								cp $(_INTERNAL_IN_DOC)/protocol.htm debian/usr/share/doc/remood
								cp $(_INTERNAL_TRUNK)/sys/remood.desktop debian/usr/share/applications
								cp $(_INTERNAL_TRUNK)/sys/remood.xpm debian/usr/share/pixmaps
								cp $(_INTERNAL_TRUNK)/sys/xmenu debian/usr/share/menu/remood
								
								chmod u+x,g+x,o+x debian/DEBIAN/postinst
								chmod u+x,g+x,o+x debian/DEBIAN/prerm
								chmod u+x,g+x,o+x debian/usr/games/remood
								chmod u+x,g+x,o+x debian/usr/games/remood.real
								
								@echo %%%% Generating Control File %%%%
								echo "Package: remood" >> debian/DEBIAN/control
								echo "Version: $(shell cat debian_helper/version)" >> debian/DEBIAN/control
								echo "Architecture: $(shell cat debian_helper/architecture)" >> debian/DEBIAN/control
								echo "Maintainer: $(shell cat debian_helper/maintainer)" >> debian/DEBIAN/control
								echo "Installed-Size: $(shell cat debian_helper/kibibytes)" >> debian/DEBIAN/control
								echo "Depends: libc6 (>= $(shell cat debian_helper/libc6ver)), libsdl1.2debian (>= $(shell cat debian_helper/libsdlver))" >> debian/DEBIAN/control
								echo "Provides: doom-engine, boom-engine" >> debian/DEBIAN/control
								echo "Recommends: doom-wad | boom-wad" >> debian/DEBIAN/control
								echo "Section: games" >> debian/DEBIAN/control
								echo "Priority: optional" >> debian/DEBIAN/control
								echo "Homepage: http://remood.org/" >> debian/DEBIAN/control
								echo "Description: `cat $(_INTERNAL_TRUNK)/sys/debinfo`" >> debian/DEBIAN/control
								
								dpkg-deb -b debian remood_$(shell cat debian_helper/strippedversion)_$(shell cat debian_helper/architecture).deb
								
								@echo %%%% Cleaning Up %%%%
								$(_INTERNAL_RMRF) debian debian_helper

.PHONY: clean-debian
clean-debian:					debian-base
								$(_INTERNAL_RMRF) remood_$(shell cat debian_helper/strippedversion)_$(shell cat debian_helper/architecture).deb
								$(_INTERNAL_RMRF) debian debian_helper
################################################################################

.PHONY:
#### SUB TARGETS ####
$(_INTERNAL_OUT_OBJ)%.o:		$(_INTERNAL_IN_SRC)/%.c
								@echo %%%%%% Compiling $< %%%%%%
								$(_INTERNAL_CC) -c $< -o $@ $(_INTERNAL_CC_FLAGS)

$(_INTERNAL_OUT_OBJ)%.o:		$(_INTERNAL_IN_ISRC)/%.c
								@echo %%%%%% Compiling $< %%%%%%
								$(_INTERNAL_CC) -c $< -o $@ $(_INTERNAL_CC_FLAGS)

$(_INTERNAL_OUT_OBJ)lnc_%.o:	$(_INTERNAL_IN_UTILLNCH)/%.c
								@echo %%%%%% Compiling $< %%%%%%
								$(_INTERNAL_CC) -c $< -o $@ $(_INTERNAL_LAUNCHERCC_FLAGS)

$(_INTERNAL_OUT_OBJ)dll_%.o:	$(_INTERNAL_IN_UTILDLL)/%.c
								@echo %%%%%% Compiling $< %%%%%%
								$(_INTERNAL_CC) -c $< -o $@ $(_INTERNAL_DLLCC_FLAGS)

$(_INTERNAL_OUT_OBJ)w32rrc.o:	$(_INTERNAL_IN_RC)/remood.rc
								@echo %%%%%% Creating resources %%%%%%
								$(_INTERNAL_WINDRES) -DMINGWMAKED --input-format=rc -o $@ $(_INTERNAL_IN_RC)/remood.rc -O coff $(_INTERNAL_RC_FLAGS)

$(_INTERNAL_OUT_OBJ)w32lrc.o:	$(_INTERNAL_IN_UTILLNCH)/w32_lnch.rc
								@echo %%%%%% Creating resources %%%%%%
								$(_INTERNAL_WINDRES) -DMINGWMAKED --input-format=rc -o $@ $(_INTERNAL_IN_UTILLNCH)/w32_lnch.rc -O coff $(_INTERNAL_RC_FLAGS)

$(_INTERNAL_OUT_OBJ)w32drc.o:	$(_INTERNAL_IN_UTILDLL)/dcdll.rc
								@echo %%%%%% Creating resources %%%%%%
								$(_INTERNAL_WINDRES) -DMINGWMAKED --input-format=rc -o $@ $(_INTERNAL_IN_UTILDLL)/dcdll.rc -O coff $(_INTERNAL_RC_FLAGS)

$(_INTERNAL_OUT_OBJROOT)/%.class:	$(_INTERNAL_IN_JAVALNCH)/%.java
									@echo %%%%%% Compiling $< %%%%%%
									$(_INTERNAL_JAVAC) $(_INTERNAL_JAVAFLAGS) $(_INTERNAL_JAVACPFLAG) $(_INTERNAL_IN_JAVALNCH) -d $(_INTERNAL_OUT_OBJROOT) $<

