#!/bin/sh

# Default compiler versions
DEFAULTGCCVER=4.6.3
DEFAULTBINVER=2.23

# Set versions we want
GCCVER=$DEFAULTGCCVER
BINVER=$DEFAULTBINVER

# Set current directory
BASEDIR="$PWD"

# Directory where compilers go
USRDIR="$BASEDIR/usr"
mkdir "$USRDIR"

# Download GCC and binutils
	# GCC first
if [ ! -d "gcc-$GCCVER" ]
then
	wget http://ftp.gnu.org/gnu/gcc/gcc-$GCCVER/gcc-core-$GCCVER.tar.bz2 -O - | tar -xjvvf -
fi
	
	# Now binutils
if [ ! -d "binutils-$BINVER" ]
then
	wget http://ftp.gnu.org/gnu/binutils/binutils-$BINVER.tar.gz -O - | tar -xzvvf -
fi

# Build binutils
rm -rf /tmp/binutils-mips
mkdir /tmp/binutils-mips
cd /tmp/binutils-mips

PATH="$USRDIR/bin:$PATH" $BASEDIR/binutils-$BINVER/configure --target mips-elf --prefix="$USRDIR" --disable-nls
make all
make install

rm -rf /tmp/binutils-mips

# Build GCC
rm -rf /tmp/gcc-mips
mkdir /tmp/gcc-mips
cd /tmp/gcc-mips

PATH="$USRDIR/bin:$PATH" $BASEDIR/gcc-$GCCVER/configure --target mips-elf --prefix="$USRDIR" --disable-nls --enable-languages=c --without-headers --disable-shared
make all-gcc
make all-target-libgcc
make install-gcc
make install-target-libgcc

rm -rf /tmp/gcc-mips

