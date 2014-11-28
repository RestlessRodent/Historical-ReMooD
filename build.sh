#!/bin/sh
# ----------------------------------------------------------------------------
# ReMooD Doom Source Port <http://remood.org/>
#   Copyright (C) 2005-2015 GhostlyDeath <ghostlydeath@remood.org>
#     For more credits, see AUTHORS.
# ----------------------------------------------------------------------------
# ReMooD is under the GNU General Public License v3 (or later), see COPYING.
# ----------------------------------------------------------------------------

# Directory of source root
EXEDIR="$(readlink -f -- "$(dirname -- "$0")")"

# List classes
echo "Listing..."
rm -f "/tmp/$$"
touch "/tmp/$$"
find "$EXEDIR/org/remood" -type f | grep '\.java' | while read -r file
do
	echo "$file" >> "/tmp/$$"
done

# Compile source code
echo "Compiling..."
TARGET=".build-remood"
rm -rf -- "$TARGET"
mkdir -p -- "$TARGET"
javac -source 1.8 -target 1.8 -d "$TARGET" -cp "$TARGET" @/tmp/$$
FAIL=$?

# Clear temporary
rm -f "/tmp/$$"

# Build JAR
if [ "$FAIL" -eq "0" ]
then
	echo "Packaging..."
	rm -f -- "remood.jar"
	jar cfe remood.jar org.remood.remood.Main -C "$TARGET" .
	EXIT=$?
	
	if [ "$FAIL" -ne "0" ]
	then
		echo "Packaging failed."
	fi
	
# Failed
else
	echo "Compilation failed."
fi

# Did it work?
exit $FAIL

