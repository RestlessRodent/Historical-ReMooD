#!/bin/sh
# ---------------------------------------------------------------------------
# ReMooD Doom Source Port <http://remood.org/>
#   Copyright (C) 2005-2016 GhostlyDeath <ghostlydeath@remood.org>
#     For more credits, see readme.mkd.
# ---------------------------------------------------------------------------
# ReMooD is under the GNU General Public License v3+, see license.mkd.
# ---------------------------------------------------------------------------

# Location of this script
__exedir="$(dirname "$0")"

# Architectures
__archs="$(uname -m) ppc powerpc i386 amd64 x86_64"

# Available VMs
__vms="server zero jamvm"

# Alternative name for architecture?
__altname()
{
	case "$1" in
		powerpc)
			echo "ppc";
			;;
		
		x86_64)
			echo "amd64";
			;;
		
		*)
			echo "$1"
		;;
	esac
}

# Alternative architectures?
__altarchs=$(for __a in $__archs; do __altname $__a; done)

# Find base directory
#/usr/lib/jvm/java-7-openjdk-powerpc/include/
#/usr/lib/jvm/java-7-openjdk-powerpc/jre/lib/ppc/server/
__findinc()
{
	for __i in $(seq 7 9)
	do
		for __a in $__archs $__altarchs
		do
			__try="/usr/lib/jvm/java-$__i-openjdk-$__a"
			if [ -d "$__try" ]
			then
				if [ -d "$__try/include" ]
				then
					echo "$__try/include"
					exit 0
				fi
			fi
		done
	done
}
__findlib()
{
	for __i in $(seq 7 9)
	do
		for __a in $__archs $__altarchs
		do
			__try="/usr/lib/jvm/java-$__i-openjdk-$__a"
			if [ -d "$__try" ]
			then
				for __b in $__archs $__altarchs
				do
					__tryb="$__try/jre/lib/$__b"
					if [ -d "$__tryb" ]
					then
						for __v in $__vms
						do
							if [ -f "$__tryb/$__v/libjvm.so" ]
							then
								echo "$__tryb/$__v/"
								exit 1
							fi
						done
					fi
				done
			fi
		done
	done
}

# Build and run it
__ji="$(__findinc)"
__jl="$(__findlib)"
echo ">> INC: $__ji"
echo ">> LIB: $__jl"
if "$__exedir/build.sh" -i "$__ji" \
	-l "$__jl" DEBUG=1
then
	LD_LIBRARY_PATH="$__jl:." \
		gdb --args bin/remood-dbg -nomouse -devparm $*
fi

