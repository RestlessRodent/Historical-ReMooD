// Emacs style mode select   -*- C++ -*- 
// -----------------------------------------------------------------------------
// ########   ###### #####   #####  ######   ######  ######
// ##     ##  ##     ##  ## ##  ## ##    ## ##    ## ##   ##
// ##     ##  ##     ##   ###   ## ##    ## ##    ## ##    ##
// ########   ####   ##    #    ## ##    ## ##    ## ##    ##
// ##    ##   ##     ##         ## ##    ## ##    ## ##    ##
// ##     ##  ##     ##         ## ##    ## ##    ## ##   ##
// ##      ## ###### ##         ##  ######   ######  ######
//                      http://remood.org/
// -----------------------------------------------------------------------------
// Project Leader:    GhostlyDeath           (ghostlydeath@gmail.com)
// Project Co-Leader: RedZTag                (jostol27@gmail.com)
// Members:           TetrisMaster512        (tetrismaster512@hotmail.com)
// -----------------------------------------------------------------------------
// Copyright (C) 1993-1996 by id Software, Inc.
// Portions Copyright (C) 1998-2000 by DooM Legacy Team.
// Portions Copyright (C) 2008-2010 The ReMooD Team..
// -----------------------------------------------------------------------------
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// -----------------------------------------------------------------------------
// DESCRIPTION: Network Header

#ifndef __NETWORK_H__
#define __NETWORK_H__

#include "master.h"

/* String Type */
typedef enum
{
	RNPST_ASCII		= 'A',
	RNPST_UTF8		= '8',
	RNPST_UTF16LE	= 'U',
	RNPST_UTF16BE	= 'u',
	RNPST_UTF32LE	= 'W',
	RNPST_UTF32BE	= 'w'
} StringType_t;

/* Flags */
typedef enum
{
	/* Compression */
	// If a client or server does not support the specified compression type, it will be dropped regardless!
	RNPF_COMPRESSIONMASK		= 0x00000007,		// Mask for compression flags
	
	RNPF_COMPRESSION_NONE		= 0x00000000,		// No Compression
	RNPF_COMPRESSION_RLE		= 0x00000001,		// RLE Compression
	RNPF_COMPRESSION_GZIP		= 0x00000002,		// GZip Compression
	RNPF_COMPRESSION_BZIP		= 0x00000003,		// BZip Compression
	RNPF_COMPRESSION_LZMA		= 0x00000004,		// LZMA Compression
	RNPF_COMPRESSION_HUFFMAN	= 0x00000005,		// Huffman Compression
	RNPF_COMPRESSION_RESERVED7	= 0x00000006,		// ??? Compression
	RNPF_COMPRESSION_RESERVED8	= 0x00000007,		// ??? Compression
	
	/* Encryption */
	// Whoever joins must know the encryption key (Using encryption is illegal in some countries!)
	RNPF_ENCRYPTIONMASK			= 0x00000038,		// Mask for encryption flags
	
	RNPF_ENCRYPTION_NONE		= 0x00000000,		// No Encryption
	RNPF_ENCRYPTION_XOR			= 0x00000008,		// XOR Encryption
	RNPF_ENCRYPTION_AES128		= 0x00000010,		// AES128 Encryption
	RNPF_ENCRYPTION_AES192		= 0x00000018,		// AES192 Encryption
	RNPF_ENCRYPTION_AES256		= 0x00000020,		// AES256 Encryption
	RNPF_ENCRYPTION_RESERVED6	= 0x00000028,		// ??? Encryption
	RNPF_ENCRYPTION_RESERVED7	= 0x00000030,		// ??? Encryption
	RNPF_ENCRYPTION_RESERVED8	= 0x00000038,		// ??? Encryption
	
	/* Misc Flags */
	RNPF_EXTENDEDPACKETSIZE		= 0x00000040,		// 32-bit Size instead of 16-bit
	RNPF_MARKER					= 0x00000080,		// Marker (sizeless)
	RNPF_HOSTIDENTITY           = 0x00000100,		// Contains a host identity (Split Screen, Fork, etc.)
	RNPF_ERRORCHECKING          = 0x00000200,		// Contains error checking to a certain extent
	RNPF_NOTIMPORTANT			= 0x00000400,		// Packet can be dropped without consequence (shit happens)
	RNPF_PADDING				= 0x00000800,		// Packet is padded to a multiple of 4 bytes
	RNPF_FORCEBYTESWAP			= 0x00001000,		// Packet deliberately violates it's specified byte order
	RNPF_DONTQUEUE				= 0x00002000,		// Don't Queue packet for Client/Server Sync
	RNPF_FORCEORDER				= 0x00004000,		// Forces ordered packet
	RNPF_TRYNORESPONSE			= 0x00008000,		// If it's viable, the client/server tries not to respond back
	RNPF_SYNCTIME				= 0x00010000,		// Contains a synchronization time
	RNPF_RESYNCORDER			= 0x00020000,		// Contains the packet number the client/server says it is
	
	/* Fun Stuff */
	RNPF_ECHO                   = 0x80000000		// Packet flags should not be taken literally
	
} RNPFlag_t;

/* ReMooD Network Protocol Packet */
typedef struct RNPPacketBase_s
{
	UInt32 ID;			// ID		(cHAT or ATcH, LE or BE, if first letter is lowercase, it's LE!)
	UInt32 Flags;		// Flags
	
	/* Misc */
	// ORDER                               = Packet Number (Unsigned 32-bit)
	//                                       if the number is less than the current packet number then this is out of order
	// RESYNCORDER                         = Unsigned 32-bit number indicating the packet number for number syncing
	// HOSTIDENTITY                        = Unsigned 16-bit number indicating player on local/remote side
	// SYNCTIME                            = Unsigned 32-bit number matching expected gametic to interpret packet
	
	/* Size */
	// MARKER                              = Nothing
	// NO COMPRESSION                      = 16-bit Unsigned Number
	// NO COMPRESSION + EXTENDEDPACKETSIZE = 32-bit Unsigned Number
	// COMPRESSION                         = Two 16-bit Unsigned Numbers (Size Uncompressed + Size Compressed)
	// COMPRESSION + EXTENDEDPACKETSIZE    = Two 32-bit Unsigned Numbers (Size Uncompressed + Size Compressed)
	
	/* Data */
	// No Data exists if MARKER is set, otherwise there is data of a certain size
	// If there is error checking, after every 8 bytes there is a byte that indicates checking and stuff
	// The first bit in the first 8 bytes will determine the error check bits
	// 0000 0001 + 0101 0100 + 1001 0010 + 1010 0111 + 0101 0010 + 1001 0101 + 0010 1000 + 0100 1001
	// will add an extra byte with the value of 10010101
} RNPPacketBase_t;

typedef struct NetworkHost_s
{
	UInt32 addr[4];
	UInt16 port;
} NetworkHost_t;

int StartServer(UInt16 Port);
int StopServer(void);
int Send(NetworkHost_t Host, UInt8* OutData, size_t OutLength, size_t OutLengthLimit);
int Receive(NetworkHost_t* Host, UInt8** InData, size_t* InLength, size_t InLengthLimit);

#endif /* __NETWORK_H__ */

