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
			# Standard release (tar.gz)
		release)
			;;
			
			# Update sys/tarfiles.txt
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
			
			# tar source to stdout
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
			
			# tar source
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
			
			# tar.gz source code
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
			
			# tar.bz2 source code
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
			
			# tar.xz source code
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
		
			# zip source code
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
			
			# Debian source package
		debian_src)
			echo "$COOLPREFIX Build Debian Source Package" 1>&2
			"$BBROOT/bb.sh" source_tgz
			
			echo "$COOLPREFIX Renaming tgz" 1>&2
			mv -v "$REMOODBASESOURCE.tgz" "$REMOODDEBIANSHORT.orig.tar.gz"
			;;
			
			# Debian package (UGLY method)
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
			
			# Debian package
		debian)
			TARGETNAME="$REMOODDEBIANNAME.deb"
			echo "$COOLPREFIX Building $TARGETNAME" 1>&2
			
			;;
	esac
	
	# Shift arguments over
	shift
done

exit 0

