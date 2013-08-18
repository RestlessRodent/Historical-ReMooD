#!/bin/sh

# Read every line from the MIPS table
I=0
cat mips.tab | while read line
do
	# Operator uses this name
	OPNAME="$line"
	
	# Now find the operator on the order table
	if grep '^'$OPNAME'#' < mips.ord > /dev/null 2> /dev/null
	then
		# Get that order again
		ORDER=$(grep '^'$OPNAME'#' < mips.ord | head -n 1)
		
		# Decode order
		# Will be something like: beq#000100#Branch#if ($s == $t) pc += i << 2
		ZNAME="$(echo $ORDER | cut -d '#' -f 1)"
		ZNUM="$(echo $ORDER | cut -d '#' -f 2)"
		ZTYPE="$(echo $ORDER | cut -d '#' -f 3)"
		ZCODE="$(echo $ORDER | cut -d '#' -f 4)"
		
		echo "$I:\t$ZNAME\t$ZTYPE\t$ZCODE"
	else
		echo "$I:\tunk$I"
	fi
	
	I=$(expr $I + 1)
done


# Read every line from the MIPS table
I=0
cat mipsa.tab | while read line
do
	# Operator uses this name
	OPNAME="$line"
	
	# Now find the operator on the order table
	if grep '^'$OPNAME'#' < mips.ord > /dev/null 2> /dev/null
	then
		# Get that order again
		ORDER=$(grep '^'$OPNAME'#' < mips.ord | head -n 1)
		
		# Decode order
		# Will be something like: beq#000100#Branch#if ($s == $t) pc += i << 2
		ZNAME="$(echo $ORDER | cut -d '#' -f 1)"
		ZNUM="$(echo $ORDER | cut -d '#' -f 2)"
		ZTYPE="$(echo $ORDER | cut -d '#' -f 3)"
		ZCODE="$(echo $ORDER | cut -d '#' -f 4)"
		
		echo "$I:\t$ZNAME\t$ZTYPE\t$ZCODE"
	else
		echo "$I:\tunk$I"
	fi
	
	I=$(expr $I + 1)
done


