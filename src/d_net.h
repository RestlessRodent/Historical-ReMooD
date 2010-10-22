// Emacs style mode select   -*- C++ -*- 
// -----------------------------------------------------------------------------
// ########   ###### #####   #####  ######   ######  ######
// ##     ##  ##     ##  ## ##  ## ##    ## ##    ## ##   ##
// ##     ##  ##     ##   ###   ## ##    ## ##    ## ##    ##
// ########   ####   ##    #    ## ##    ## ##    ## ##    ##
// ##    ##   ##     ##         ## ##    ## ##    ## ##    ##
// ##     ##  ##     ##         ## ##    ## ##    ## ##   ##
// ##      ## ###### ##         ##  ######   ######  ######
//                      http://remood.sourceforge.net/
// -----------------------------------------------------------------------------
// Project Leader:    GhostlyDeath           (ghostlydeath@gmail.com)
// Project Co-Leader: RedZTag                (jostol27@gmail.com)
// Members:           Demyx                  (demyx@endgameftw.com)
//                    Dragan                 (poliee13@hotmail.com)
// -----------------------------------------------------------------------------
// Copyright (C) 1993-1996 by id Software, Inc.
// Portions Copyright (C) 1998-2000 by DooM Legacy Team.
// Portions Copyright (C) 2008-2009 The ReMooD Team..
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
// DESCRIPTION: Networking stuff.
//              part of layer 4 (transport) (tp4) of the osi model
//              assure the reception of packet and proceed a checksums

#ifndef __D_NET__
#define __D_NET__

#include "i_net.h"

//
// Network play related stuff.
// There is a data struct that stores network
//  communication related stuff, and another
//  one that defines the actual packets to
//  be transmitted.
//

// Max computers in a game.
#define MAXNETNODES             32
#define BROADCASTADDR           MAXNETNODES	// added xx/5/99: can use broadcast now

#define STATLENGTH  (TICRATE*2)

// stat of net
extern int getbps, sendbps;
extern float lostpercent, duppercent, gamelostpercent;
extern int packetheaderlength;
extern int getbytes;
extern INT64 sendbytes;			// realtime updated 

// if reliable return true if packet sent, 0 else
boolean D_CheckNetGame(void);
void D_CloseConnection(void);

/*******************************************************************************
**************************** ReMooD Network Protocol ***************************
*******************************************************************************/

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

/* Packet Handler Hashing */
// Packet ID composition will either be from 0x41 to 0x5A or from 0x61 to 0x7A
// Upper Case A > 0x41 > 0100 0001
// Lower Case a > 0x61 > 0110 0001
// Upper Case Z > 0x5A > 0101 1010
// Lower Case z > 0x7A > 0111 1010
// Chars go from 0 to 256 (2^8), top 2 bits don't matter at all! goes down to 0 to 64 (2^6)
// 0100 0001 >> xx00 0001 ('A')
// 3rd bit determines if it's upper case or lower case leaving 0 to 32 (2^5)
// Since case determines endieness of the packets, the case bit can be cut off
// So there could be 32 arrays that contain lists of handlers based on the first character
// There are only 26 letters in the alphabet though, so if a number is greater than Z it can become Z
// The resulting number could be subtracted by 1 so 'A' is 0x00 and 'Z' is 0x19
// 0x19 in binary is 1 1001 and 0x20 is 10 0000 so to see if a letter is valid, we can just check for 0x20
// To optimize, ReMooD could determine new ordering based on how many times a packet is requested and such
typedef int (*RNPHandler_t)(RNPPacketBase_t* BasePtr, UInt16 Host, UInt8* Data, size_t Size);

typedef struct HandlerHolder_s
{
	UInt32 Rest;			// Rest of the code
	int Num;				// Priority Number
	int Count;				// Call count
	RNPHandler_t* Handler;	// Handler
	
	/* Links if ever needed */
	struct HandlerHolder_s* Next;
	struct HandlerHolder_s* Prev;
} HandlerHolder_t;

/* Network Host */
typedef struct NetworkHost_s
{
	UInt32 ip[4];
	UInt16 Port;
} NetworkHost_t;

/* Network Player */
typedef struct NetworkPlayer_s
{
	/* Identity */
	UInt8 LocalID;						// Local Player Number
	UInt16 NetworkID;					// Network ID
	
	/* Look */
	wchar_t Name[MAXPLAYERNAME];		// Name
	wchar_t Skin[MAXPLAYERNAME];		// Skin
	UInt8 Color;						// Color
	
	/* Timing */
	UInt32 LocalGameTic;				// Local Game Tic
	UInt32 RemoteGameTic;				// Remote Game Tic
	UInt32 ToTime;						// Average time it takes to get there (in tics)
	UInt32 FromTime;					// Average time it takes to get here (in tics)
	UInt32 MSToTime;					// Actual ping (time to there)
	UInt32 MSFromTime;					// Virtual ping (time to here)
		// There will be 2 parts of the ping indicator, Actual Ping and Virtual Ping (ap/vp)
		// Tic Based Ping will be ((ToTime + FromTime) * 28)
		// Which means in game pings will be: 0, 28, 56, 84, 112, 140, 168, 196, 224, 252, 280, 308, 336,
		//                                    364, 392, 420, 448, 476, 504, 532, and 560
		//                                    If anyone has more than 560VP then they will be lagged even more since
		//                                    prediction only goes up to 10 tics there and back
		// Actual ping will be the actual amount of time it takes to get there, this will hover around virtual ping
	
	/* Host */
	UInt32 SizeLimit;					// Limits packet size (if the limit is reached then the server will go snip
										// on the packets marked NOTIMPORTANT).
	NetworkHost_t Host;					// Host
} NetworkPlayer_t;

extern HandlerHolder_t* HandlerHash[26];
extern NetworkPlayer_t NetPlayers[MAXPLAYERS];

#endif

