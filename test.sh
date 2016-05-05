#!/bin/sh
# ---------------------------------------------------------------------------
# ReMooD Doom Source Port <http://remood.org/>
#   Copyright (C) 2005-2016 GhostlyDeath <ghostlydeath@remood.org>
#     For more credits, see readme.mkd.
# ---------------------------------------------------------------------------
# ReMooD is under the GNU General Public License v3+, see license.mkd.
# ---------------------------------------------------------------------------

# Major java version
__majorversions="7 8"

# Architectures
__architectures="$(uname -m) powerpc i386 amd64 x86_64"

# Available VMs
__vms="server zero jamvm"

# Find Java include header
__findinclude()
{
	for __jv in $__majorversions
	do
		for __ja in $__architectures
		do
			__p="/usr/lib/jvm/java-$__jv-openjdk-$__ja/include/"
			if [ -f "$__p/jni.h" ]
			then
				echo "$__p"
				exit 1
			fi
		done
	done
}

# Find library directory
__findlib()
{
	for __jv in $__majorversions
	do
		for __ja in $__architectures
		do
			for __vm in $__vms
			do
				__p="/usr/lib/jvm/java-$__jv-openjdk-$__ja/jre/lib/$__ja/$__vm/"
				if [ -f "$__p/libjvm.so" ]
				then
					echo "$__p"
					exit 1
				fi
			done
		done
	done
}

# Build and run it
if make "JAVA_INCLUDE=$(__findinclude)" "JAVA_LIB=$(__findlib)" DEBUG=1
then
	gdb --args bin/remood-dbg -nomouse -devparm
fi

