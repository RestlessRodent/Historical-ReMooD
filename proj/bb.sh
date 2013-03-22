#!/bin/sh
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
# Buildbot Helper, or just an easy as hell to use builder script.

### PRE-CHECKS ###

### COOL STUFF ###
# Building Stuff
BBROOT='./proj'
BBREMOOD='.'

# Cool
COOLPREFIX=' $> '
COOLERROR=' !> '

# Versioning
REMOODVERSION="$(cat version)"
REMOODVERSIONSTRIP="$(sed 's/\.//g' < version)"
REMOODARCH="$(dpkg --print-architecture)"

# File names
REMOODBASENAME="remood"
REMOODBASESOURCE="remood-src-$REMOODVERSIONSTRIP"
REMOODDEBIANSHORT="remood_$REMOODVERSIONSTRIP"
REMOODDEBIANNAME="${REMOODDEBIANSHORT}_$REMOODARCH"

### CHECK FOR HG ###
if [ "$HG" != "ignore" ]
then
	if ! which hg > /dev/null 2> /dev/null || ! hg status "$BBREMOOD" > /dev/null 2> /dev/null
	then
		HG=""
	else
		HG="ok"
	fi
fi

### LOOP ARG BUILDING ###
for target in $*
do
	# Which package is being built?
	case $target
	in
###############################################################################
### Standard release (tar.gz)
###############################################################################
		release)
			;;
			
###############################################################################
### Update sys/tarfiles.txt
###############################################################################
		system_tarfiles)
			# Print message
			echo "$COOLPREFIX Updating sys/tarfiles.txt" 1>&2
			
			# Is hg ok?
			if [ "$HG" != "ok" ]
			then
				echo "$COOLERROR Mercurial not found!" 1>&2
			fi
			
			# Clear file
			rm -f "$BBREMOOD/sys/tarfiles.txt"
			
			# For every file
			find "$BBREMOOD" -type f | grep -v '\/\.hg\/' | sed 's/^\.\///' | while read file
			do
				# Check if the file is tracked
				if hg status "$file" -m -a -r -d -c 2> /dev/null | grep '^[MARC\!\?]\ ' > /dev/null 2> /dev/null
				then
					echo "$file" >> "$BBREMOOD/sys/tarfiles.txt"
				fi
			done
			
			# Sort it
			sort < "$BBREMOOD/sys/tarfiles.txt" > /tmp/$$
			mv /tmp/$$ "$BBREMOOD/sys/tarfiles.txt"
			
			# Print message
			echo "$COOLPREFIX Done" 1>&2
			;;
			
###############################################################################
### tar source to stdout
###############################################################################
		source_tar_stdout)
			echo "$COOLPREFIX tar to stdout" 1>&2
			
			# Create directory to put symlinks in
			mkdir -p "tsrc$$/$REMOODBASESOURCE" > /dev/null 2> /dev/null
			
			#
			(while read line
			do
				# Create directory to place symlink in
				mkdir -p "tsrc$$/$REMOODBASESOURCE/$(dirname "$line")"
				ln -s "$(readlink -m "$BBREMOOD/$line")" "tsrc$$/$REMOODBASESOURCE/$line"
			done) < "sys/tarfiles.txt" > /dev/null 2> /dev/null
			
			# Go into tree and create archive
			(cd "tsrc$$"; tar -chvvf - "$REMOODBASESOURCE") 2> /dev/null
			
			# Delete source tree
			rm -rf "tsrc$$" > /dev/null 2> /dev/null
			;;
			
###############################################################################
### tar source
###############################################################################
		source_tar)
			TARGETNAME="$REMOODBASESOURCE.tar"
			echo "$COOLPREFIX Building $TARGETNAME" 1>&2
			
			# If hg exists, use that instead
			if [ "$HG" = "ok" ]
			then
				hg archive -p "$REMOODBASESOURCE/" -t tar "$TARGETNAME" 1>&2
			
			# Otherwise, use harder method
			else
				# Read tar from std and make archive
				"$BBROOT/bb.sh" source_tar_stdout > "$TARGETNAME"
			fi
			;;
			
###############################################################################
### tar.gz source code
###############################################################################
		source_tgz)
			TARGETNAME="$REMOODBASESOURCE.tgz"
			echo "$COOLPREFIX Building $TARGETNAME" 1>&2
			
			# If hg exists, use that instead
			if [ "$HG" = "ok" ]
			then
				hg archive -p "$REMOODBASESOURCE/" -t tgz "$TARGETNAME" 1>&2
			
			# Otherwise, use harder method
			else
				# Read tar from std and make archive
				"$BBROOT/bb.sh" source_tar_stdout | gzip -9 -c - > "$TARGETNAME"
			fi
			;;
			
###############################################################################
### tar.bz2 source code
###############################################################################
		source_tbz)
			TARGETNAME="$REMOODBASESOURCE.tbz"
			echo "$COOLPREFIX Building $TARGETNAME" 1>&2
			
			# If hg exists, use that instead
			if [ "$HG" = "ok" ]
			then
				hg archive -p "$REMOODBASESOURCE/" -t tgz "$TARGETNAME" 1>&2
			
			# Otherwise, use harder method
			else
				# Read tar from std and make archive
				"$BBROOT/bb.sh" source_tar_stdout | bzip2 -9 -c -  > "$TARGETNAME"
			fi
			;;
			
###############################################################################
### tar.xz source code
###############################################################################
		source_txz)
			TARGETNAME="$REMOODBASESOURCE.txz"
			echo "$COOLPREFIX Building $TARGETNAME" 1>&2
			
			# If hg exists, use that instead
			if [ "$HG" = "ok" ]
			then
				# hg lacks xz support so first create tar archive
				"$BBROOT/bb.sh" source_tar
				
				# Recompress as xz
				xz -9 -c - < "$REMOODBASESOURCE.tar" > "$TARGETNAME"
			
			# Otherwise, use harder method
			else
				# Read tar from std and make archive
				"$BBROOT/bb.sh" source_tar_stdout | xz -9 -c -  > "$TARGETNAME"
			fi
			;;
		
###############################################################################
### zip source code
###############################################################################
		source_zip)
			TARGETNAME="$REMOODBASESOURCE.zip"
			echo "$COOLPREFIX Building $TARGETNAME" 1>&2
			
			# If hg exists, use that instead
			if [ "$HG" = "ok" ]
			then
				hg archive -p "$REMOODBASESOURCE/" -t zip "$TARGETNAME" 1>&2
			
			# Otherwise, use harder method
			else
				# Oops
				echo "$COOLPREFIX ZIP wihtout hg not yet supported." 1>&2 
			fi
			;;
			
###############################################################################
### Debian source package
###############################################################################
		debian_src)
			echo "$COOLPREFIX Build Debian Source Package" 1>&2
			"$BBROOT/bb.sh" source_tgz
			
			echo "$COOLPREFIX Renaming tgz" 1>&2
			mv -v "$REMOODBASESOURCE.tgz" "$REMOODDEBIANSHORT.orig.tar.gz"
			;;
			
###############################################################################
### Debian package (UGLY method)
###############################################################################
		debian_ugly)

			TARGETNAME="$REMOODDEBIANNAME.deb"
			echo "$COOLPREFIX Building $TARGETNAME" 1>&2
			
			echo "$COOLPREFIX Cleaning default source" 1>&2
			if ! make clean USEINTERFACE=sdl
			then
				echo "$COOLPREFIX Failed to clean" 1>&2
				exit 1
			fi
			
			echo "$COOLPREFIX Building default source" 1>&2
			if ! make USEINTERFACE=sdl
			then
				echo "$COOLPREFIX Failed to build" 1>&2
				exit 1
			fi
			
			echo "$COOLPREFIX Clearing old debian dir (if any)" 1>&2
			rm -rvf ./debian/
			
			echo "$COOLPREFIX Creating new tree" 1>&2
			mkdir -p ./debian/DEBIAN
			mkdir -p ./debian/usr/games/
			mkdir -p ./debian/usr/share/applications
			mkdir -p ./debian/usr/share/doc/remood
			mkdir -p ./debian/usr/share/games/doom
			mkdir -p ./debian/usr/share/menu
			mkdir -p ./debian/usr/share/pixmaps
			
			echo "$COOLPREFIX Filling binary tree" 1>&2
			cp -v "$BBREMOOD/sys/remood.desktop" ./debian/usr/share/applications
			cp -v "./bin/remood" ./debian/usr/games/remood.real
			cp -v "$BBREMOOD/sys/wrap.sh" ./debian/usr/games/remood
			cp -v "$BBREMOOD/bin/remood.wad" ./debian/usr/share/games/doom
			cp -v "$BBREMOOD/sys/xmenu" ./debian/usr/share/menu
			cp -v "$BBREMOOD/sys/remood.xpm" ./debian/usr/share/pixmaps
			
			cp -v "$BBREMOOD/sys/debpostinst" ./debian/DEBIAN/postinst
			chmod 755 ./debian/DEBIAN/postinst
			cp -v "$BBREMOOD/sys/debprerm" ./debian/DEBIAN/prerm
			chmod 755 ./debian/DEBIAN/prerm
			
			echo "$COOLPREFIX Creating control file" 1>&2
			DEBIANCONTROLFILE="./debian/DEBIAN/control"
			echo "Package: remood" > $DEBIANCONTROLFILE
			echo "Architecture: $REMOODARCH" >> $DEBIANCONTROLFILE
			echo "Version: $REMOODVERSION" >> $DEBIANCONTROLFILE
			echo "Maintainer: GhostlyDeath (ghostlydeath@remood.org)" >> $DEBIANCONTROLFILE
			# Installed-Size
			echo "Depends: libc6 (>= `dpkg-query -W libc6 | sed 's/^[^\ \t]*[\ \t]*\(.*\)$/\1/g'`), libsdl1.2debian (>= `dpkg-query -W libsdl1.2debian | sed 's/^[^\ \t]*[\ \t]*\(.*\)$/\1/g'`)" >> $DEBIANCONTROLFILE
			echo "Provides: doom-engine, boom-engine, heretic-engine" >> $DEBIANCONTROLFILE
			echo "Recommends: doom-wad | boom-wad | heretic-wad" >> $DEBIANCONTROLFILE
			echo "Section: games" >> $DEBIANCONTROLFILE
			echo "Priority: optional" >> $DEBIANCONTROLFILE
			echo "Homepage: http://remood.org/" >> $DEBIANCONTROLFILE
			echo "Description: `cat "$BBREMOOD/sys/debinfo"`" >> $DEBIANCONTROLFILE
			
			echo "$COOLPREFIX Compiling package" 1>&2
			dpkg-deb -b debian "$REMOODDEBIANNAME.deb"
			;;
			
###############################################################################
### Debian package
###############################################################################
		debian)
			TARGETNAME="$REMOODDEBIANNAME.deb"
			echo "$COOLPREFIX Building $TARGETNAME" 1>&2
			
			;;
			
###############################################################################
### Win32 Compiler
###############################################################################
		win32_findcc)
			# Which prefix works?
				# Win64 name last (Win64 does not work on 98)
			for PREFIX in i686-pc-mingw32 i686-mingw32 i586-mingw32msvc i586-pc-mingw32msvc i686-w64-mingw32
			do
				if which "${PREFIX}-gcc" > /dev/null 2> /dev/null
				then
					echo "${PREFIX}-"
					exit 0
				fi
			done
			
			# Fallback
			echo ""
			;;
			
###############################################################################
### Win64 Compiler
###############################################################################
		win64_findcc)
			# Which prefix works?
			# These are all the same CCs, second is comp, last is Debian screwy name
			for PREFIX in x86_64-w64-mingw32 x86_64-pc-mingw32 amd64-mingw32msvc
			do
				if which "${PREFIX}-gcc" > /dev/null 2> /dev/null
				then
					echo "${PREFIX}-"
					exit 0
				fi
			done
			
			# Fallback
			echo ""
			;;
			
###############################################################################
### Win32/Allegro Binary
###############################################################################
		win32_allegro)
			echo "$COOLPREFIX Building Win32 Allegro Binary" 1>&2
			
			WINGCC="`"$BBROOT/bb.sh" win32_findcc`"
			echo "$COOLPREFIX Using $WINGCC" 1>&2
			
			# Check if allegw32 needs to be extracted
			if  [ ! -d "allegw32" ]
			then
				# Download from remood.org
				if [ ! -f "allegw32.tar.gz" ]
				then
					wget -c http://remood.org/downloads/allegw32.tar.gz -O allegw32.tar.gz
				fi
				
				# Extract
				if ! tar -xvvf "allegw32.tar.gz"
				then
					echo "$COOLPREFIX Failed to extract Allegro/Win32" 1>&2
					exit 1
				fi
			fi
			
			# Compile
			make clean USEINTERFACE=allegro TOOLPREFIX="$WINGCC"
			if ! make USEINTERFACE=allegro TOOLPREFIX="$WINGCC" ALLEGRO_INCLUDE=allegw32/include ALLEGRO_LIB=allegw32/lib
			then
				echo "$COOLPREFIX Failed" 1>&2
				exit 1
			fi
			
			# Zip
			mkdir -p "$$/"
			cp -v "bin/remood.exe" "$$/"
			cp -v "$BBREMOOD/bin/remood.wad" "$$/"
			cp -v "allegw32/bin/alleg42.dll" "$$/"
			cp -v "$BBREMOOD/doc/manual.pdf" "$$/"
			cp -v "$BBREMOOD/AUTHORS" "$$/"
			cp -v "$BBREMOOD/LICENSE" "$$/"
			cp -v "$BBREMOOD/version" "$$/"
			
			# Go into dir
			cd "$$/"
			
			# Convert to DOS format
			unix2dos -o AUTHORS LICENSE version
			
			# Zip files into an archive
			rm -f "../remood_${REMOODVERSIONSTRIP}_win32.zip"
			zip "../remood_${REMOODVERSIONSTRIP}_win32.zip" *
			
			# Get back out
			cd "../"
			rm -rf "$$"
			
			;;
			
###############################################################################
### Win32/SDL Binary
###############################################################################
		win32_sdl)
			echo "$COOLPREFIX Building Win32 SDL Binary" 1>&2
			
			# Find GCC
			WINGCC="`"$BBROOT/bb.sh" win32_findcc`"
			echo "$COOLPREFIX Using $WINGCC" 1>&2
			
			
			# Check if sdlw32 needs to be extracted
			if  [ ! -d "SDL-1.2.15" ]
			then
				# Download from remood.org
				if [ ! -f "SDL-devel-1.2.15-mingw32.tar.gz" ]
				then
					wget -c http://remood.org/downloads/SDL-devel-1.2.15-mingw32.tar.gz -O SDL-devel-1.2.15-mingw32.tar.gz
				fi
				
				# Extract
				if ! tar -xvvf "SDL-devel-1.2.15-mingw32.tar.gz"
				then
					echo "$COOLPREFIX Failed to extract SDL/Win32" 1>&2
					exit 1
				fi
			fi
			
			# Compile
			make clean USEINTERFACE=sdl TOOLPREFIX="$WINGCC"
			if ! make USEINTERFACE=sdl TOOLPREFIX="$WINGCC" SDL_INCLUDE=SDL-1.2.15/include/SDL SDL_LIB=SDL-1.2.15/lib OPENGL_LDFLAGS="-lopengl32"
			then
				echo "$COOLPREFIX Failed" 1>&2
				exit 1
			fi
			
			# Zip
			mkdir -p "$$/"
			cp -v "bin/remood.exe" "$$/"
			cp -v "$BBREMOOD/bin/remood.wad" "$$/"
			cp -v "SDL-1.2.15/bin/SDL.dll" "$$/"
			cp -v "$BBREMOOD/doc/manual.pdf" "$$/"
			cp -v "$BBREMOOD/AUTHORS" "$$/"
			cp -v "$BBREMOOD/LICENSE" "$$/"
			cp -v "$BBREMOOD/version" "$$/"
			
			# Go into dir
			cd "$$/"
			
			# Convert to DOS format
			unix2dos -o AUTHORS LICENSE version
			
			# Zip files into an archive
			rm -f "../remood_${REMOODVERSIONSTRIP}_win32.zip"
			zip "../remood_${REMOODVERSIONSTRIP}_win32.zip" *
			
			# Get back out
			cd "../"
			rm -rf "$$"
			
			;;
			
###############################################################################
### Default Win32 Binary
###############################################################################
		win32)
			echo "$COOLPREFIX Building Win32 Default Binary" 1>&2
			"$BBROOT/bb.sh" win32_allegro
			;;
			
###############################################################################
### Nintendo Wii
###############################################################################
		wii)
			# Message
			echo "$COOLPREFIX Building Wii Package" 1>&2
			
			# Devkitpro Root
			if [ "$DEVKITPPC" = "" ]
			then
				DEVKITPPC="/opt/devkitppc/devkitPPC"
			fi
			
			# Compile
			if ! make CDEFFLAGS="-D__REMOOD_WII" CC="$DEVKITPPC/bin/powerpc-eabi-gcc" OPENGL_LDFLAGS="-D__REMOOD_OPENGL_CANCEL" EXESUFFIX=".elf"
			then
				echo "$COOLPREFIX Build failed" 1>&2
				exit 1
			fi
			
			# Convert ELF to DOL
			
			;;
			
###############################################################################
### GCW Zero
###############################################################################
		gcw)
			# Message
			echo "$COOLPREFIX Building GCW Package" 1>&2
			
			# Basic Root
			${GCWTCROOT:="/opt/gcw0-toolchain/"}
			
			# Sanity Checks
				# SquashFS
			if ! which mksquashfs 2> /dev/null > /dev/null
			then
				echo "$COOLPREFIX You need mksquashfs" 1>&2
				exit 1
			fi
				
				# GCW Root
			if [ ! -d "$GCWTCROOT" ]
			then
				echo "$COOLPREFIX You need the GCW chain, set GCWTCROOT to where it exists (default /opt/gcw0-toolchain/)" 1>&2
				exit 1
			fi
			
			# Build
			if ! make CC="$GCWTCROOT/usr/bin/mipsel-gcw0-linux-uclibc-gcc" USEINTERFACE=sdl CFLAGS="-I$GCWTCROOT/usr/mipsel-gcw0-linux-uclibc/sysroot/usr/include/SDL -D__REMOOD_OPENGL_CANCEL -D__REMOOD_MODEL=1 -D__REMOOD_NOALSAMIDI" OPENGL_LDFLAGS="-D__REMOOD_OPENGL_CANCEL" EXESUFFIX=".elf" CONFIGPREFIX="I$GCWTCROOT/usr/mipsel-gcw0-linux-uclibc/sysroot/usr/bin/"
			then
				echo "$COOLPREFIX Build failed" 1>&2
				exit 1
			fi
			
			# Setup OPK
			mkdir -p $$/
			cd $$/
			
			# Copy Files we want
			cp ../sys/gcw.dsk default.gcw0.desktop
			cp ../sys/gcw.png remood.png
			cp ../bin/remood.elf remood.elf
			cp ../bin/remood.wad remood.wad
			cp ../doc/AUTHORS AUTHORS
			cp ../doc/LICENSE LICENSE
			cp ../doc/manual.pdf manual.pdf
			cp ../sys/gcw.txt remood.txt
			
			# version
			echo "Version=$REMOODVERSION" >> default.gcw0.desktop
			
			# Make sure everything is readable
			chmod uog+r *
			chmod uog+x remood.elf
			
			cd ..
			
			# Squash It
			rm -f remood_${REMOODVERSIONSTRIP}_gcw.opk
			mksquashfs $$ remood_${REMOODVERSIONSTRIP}_gcw.opk -force-uid 1000 -force-gid 1000
			
			# Done
			rm -rf "$$"
			;;
			
###############################################################################
### Palm OS
###############################################################################
		palmos)
			echo "$COOLPREFIX Building Palm OS PRC" 1>&2
			
			# Download Floating Point Object
			echo "$COOLPREFIX Checking for GCCs floatlib" 1>&2
			if [ ! -f "arm-palmos-gcc-floatlib.o" ]
			then
				# Download from remood.org
				wget -c http://remood.org/downloads/arm-palmos-gcc-floatlib.o -O arm-palmos-gcc-floatlib.o
			fi
			
			# Download Boot Lib
			echo "$COOLPREFIX Checking for libarmboot.a" 1>&2
			if [ ! -f "libarmboot.a" ]
			then
				# Download from remood.org and gunzip
				wget -c http://remood.org/downloads/libarmboot.a.gz -O libarmboot.a.gz
				gunzip libarmboot.a.gz
			fi
			
			# Check if peal needs to be extracted
			echo "$COOLPREFIX Checking for PEAL" 1>&2
			if  [ ! -d "peal-2005_4_14" ]
			then
				# Download from remood.org
				if [ ! -f "peal-2005_4_14.tar.gz" ]
				then
					wget -c http://remood.org/downloads/peal-2005_4_14.tar.gz -O peal-2005_4_14.tar.gz
				fi
				
				# Extract
				if ! tar -xvvf "peal-2005_4_14.tar.gz"
				then
					echo "$COOLPREFIX Failed to extract PEAL" 1>&2
					exit 1
				fi
				
				# Fix Compilation Problems
					# Extra Qualifier
				sed 's/Relocation::asElf/asElf/' < peal-2005_4_14/postlink/relocation.h > $$
				mv $$ peal-2005_4_14/postlink/relocation.h
				
					# Callocs and Mallocs Missing
				sed 's/#include <stdint.h>/#include <stdlib.h>\n#include <stdint.h>/' < peal-2005_4_14/postlink/got.h > $$
				mv $$ peal-2005_4_14/postlink/got.h
				sed 's/#include <string.h>/#include <stdlib.h>\n#include <string.h>/' < peal-2005_4_14/postlink/stringtable.h > $$
				mv $$ peal-2005_4_14/postlink/stringtable.h
				
					# perror
				sed 's/#include <unistd.h>/#include <stdio.h>\n#include <unistd.h>/' < peal-2005_4_14/postlink/image.cc > $$
				mv $$ peal-2005_4_14/postlink/image.cc
				
					# find (which is in <algorithm>
				sed 's/#include <string>/#include <algorithm>\n#include <string>/' < peal-2005_4_14/postlink/symbol.cc > $$
				mv $$ peal-2005_4_14/postlink/symbol.cc
				sed 's/#include <string>/#include <algorithm>\n#include <string>/' < peal-2005_4_14/postlink/section.cc > $$
				mv $$ peal-2005_4_14/postlink/section.cc
				
			fi
			
			# Check if PARM needs to be extracted
			echo "$COOLPREFIX Checking for PARM" 1>&2
			if  [ ! -d "parm" ]
			then
				# Download from remood.org
				if [ ! -f "parm.zip" ]
				then
					wget -c http://remood.org/downloads/parm.zip -O parm.zip
				fi
				
				# Extract
				if ! unzip parm.zip
				then
					echo "$COOLPREFIX Failed to extract PARM" 1>&2
					exit 1
				else
					# Lowercase it
					mv PARM parm
				fi
			fi
			
			# Compile PEAL
			echo "$COOLPREFIX Compiling PEAL" 1>&2
			cd peal-2005_4_14
			cd postlink
			make clean
			make
			
			# Test it
			./peal-postlink
			
			# Get out of the directory
			cd ..
			cd ..
			
			# Compile ReMooD Now
			echo "$COOLPREFIX Now Compiling..." 1>&2
			make clean USEINTERFACE=palmos TOOLPREFIX="arm-palmos-"
			if ! make USEINTERFACE=palmos TOOLPREFIX="arm-palmos-" CFLAGS="-Iparm -Ipeal-2005_4_14/arm" LDFLAGS="-lc libarmboot.a"
			then
				echo "$COOLPREFIX Failed" 1>&2
				exit 1
			fi
			
			;;
			
###############################################################################
### Test Suite
###############################################################################
		test)
			echo "$COOLPREFIX Prepare test with build" 1>&2
			make clean USEINTERFACE=headless DEBUG=1 > /dev/null 2> /dev/null
			
			if ! make USEINTERFACE=headless DEBUG=1 2> /dev/null
			then
				echo "$COOLPREFIX Make failed..." 1>&2
				exit 1
			fi
			
			echo "$COOLPREFIX Running tests..." 1>&2
			
			# Start and quite test
			echo -n "$COOLPREFIX TEST: START AND QUIT: " 1>&2
			if ! bin/remood-dbg -devparm ++quit > /dev/null 2> /dev/null
			then
				echo "FAILED ($?)" 1>&2
			else
				echo "PASSED ($?)" 1>&2
			fi
			
			;;
	esac
	
	# Shift arguments over
	shift
done

exit 0

