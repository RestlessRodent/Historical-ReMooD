// -*- Mode: C++; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-
// -----------------------------------------------------------------------------
//         :oCCCCOCoc.
//     .cCO8OOOOOOOOO8Oo:
//   .oOO8OOOOOOOOOOOOOOOCc
//  cO8888:         .:oOOOOC.                                                TM
// :888888:   :CCCc   .oOOOOC.     ###      ###                    #########
// C888888:   .ooo:   .C########   #####  #####  ######    ######  ##########
// O888888:         .oO###    ###  #####  ##### ########  ######## ####    ###
// C888888:   :8O.   .C##########  ### #### ### ##    ##  ##    ## ####    ###
// :8@@@@8:   :888c   o###         ### #### ### ########  ######## ##########
//  :8@@@@C   C@@@@   oo########   ###  ##  ###  ######    ######  #########
//    cO@@@@@@@@@@@@@@@@@Oc0
//      :oO8@@@@@@@@@@Oo.
//         .oCOOOOOCc.                                      http://remood.org/
// -----------------------------------------------------------------------------
// Copyright (C) 2011-2012 GhostlyDeath <ghostlydeath@remood.org>.
// -----------------------------------------------------------------------------
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 3
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// -----------------------------------------------------------------------------
// DESCRIPTION: Block Transport API

#ifndef __D_BLOCK_H__
#define __D_BLOCK_H__

/***************
*** INCLUDES ***
***************/

#include "doomtype.h"
#include "d_net.h"

/****************
*** CONSTANTS ***
****************/

#define RBLOCKBUFSIZE					128		// Block buffer size

/* D_RBSStreamFlags_t -- Flags for streams */
typedef enum D_RBSStreamFlags_s
{
	DRBSSF_OVERWRITE				= 0x0001,	// Overwrite existing file
	DRBSSF_READONLY					= 0x0002,	// Create Read Only Stream
} D_RBSStreamFlags_t;

/* D_RBSStreamIOCtl_t -- IOCtl Command for stream */
typedef enum D_RBSStreamIOCtl_e
{
	DRBSIOCTL_ISPERFECT,						// Read: Is perfect?
	
	NUMDRBSSTREAMIOCTL
} D_RBSStreamIOCtl_t;

/*****************
*** STRUCTURES ***
*****************/

/* D_RBlockStream_t -- Remote block stream */
typedef struct D_RBlockStream_s
{
	/* Info */
	void* Data;									// Private Data
	uint32_t Flags;								// Flags (if ever needed)
	
	/* Current Block */
	char BlkHeader[4];							// Block identifier
	uint8_t* BlkData;							// Data
	size_t BlkSize;								// Block Size
	size_t BlkBufferSize;						// Size of buffer
	size_t ReadOff;								// Read Offset
	bool Marked;								// Marked?
	
	/* Functions */
	size_t (*RecordF)(struct D_RBlockStream_s* const a_Stream);
	bool (*PlayF)(struct D_RBlockStream_s* const a_Stream);
	bool (*FlushF)(struct D_RBlockStream_s* const a_Stream);
	
	size_t (*NetRecordF)(struct D_RBlockStream_s* const a_Stream, I_HostAddress_t* const a_Host);
	bool (*NetPlayF)(struct D_RBlockStream_s* const a_Stream, I_HostAddress_t* const a_Host);
	
	void (*DeleteF)(struct D_RBlockStream_s* const a_Stream);
	bool (*IOCtlF)(struct D_RBlockStream_s* const a_Stream, const D_RBSStreamIOCtl_t a_IOCtl, int32_t* a_DataP);
	
	/* Stream Stat */
	uint32_t StatBlock[2];						// Block stats
	uint32_t StatBytes[2];						// Byte stats
} D_RBlockStream_t;

/****************
*** FUNCTIONS ***
****************/

struct WL_EntryStream_s;

D_RBlockStream_t* D_RBSCreateLoopBackStream(void);
D_RBlockStream_t* D_RBSCreateWLStream(struct WL_EntryStream_s* const a_Stream);
D_RBlockStream_t* D_RBSCreateFileStream(const char* const a_PathName, const uint32_t a_Flags);
D_RBlockStream_t* D_RBSCreateNetStream(I_NetSocket_t* const a_NetSocket);
D_RBlockStream_t* D_RBSCreatePerfectStream(D_RBlockStream_t* const a_Wrapped);
void D_RBSCloseStream(D_RBlockStream_t* const a_Stream);

void __REMOOD_DEPRECATED D_RBSStatStream(D_RBlockStream_t* const a_Stream, uint32_t* const a_ReadBk, uint32_t* const a_WriteBk, uint32_t* const a_ReadBy, uint32_t* const a_WriteBy);
void __REMOOD_DEPRECATED D_RBSUnStatStream(D_RBlockStream_t* const a_Stream);
bool __REMOOD_DEPRECATED D_RBSMarkedStream(D_RBlockStream_t* const a_Stream);

bool D_RBSStreamIOCtl(D_RBlockStream_t* const a_Stream, const D_RBSStreamIOCtl_t a_IOCtl, int32_t* a_DataP);

bool D_RBSCompareHeader(const char* const a_A, const char* const a_B);
bool D_RBSBaseBlock(D_RBlockStream_t* const a_Stream, const char* const a_Header);
bool D_RBSRenameHeader(D_RBlockStream_t* const a_Stream, const char* const a_Header);

bool D_RBSPlayBlock(D_RBlockStream_t* const a_Stream, char* const a_Header);
void D_RBSRecordBlock(D_RBlockStream_t* const a_Stream);

bool D_RBSPlayNetBlock(D_RBlockStream_t* const a_Stream, char* const a_Header, I_HostAddress_t* const a_Host);
void D_RBSRecordNetBlock(D_RBlockStream_t* const a_Stream, I_HostAddress_t* const a_Host);

bool D_RBSFlushStream(D_RBlockStream_t* const a_Stream);

size_t D_RBSWriteChunk(D_RBlockStream_t* const a_Stream, const void* const a_Data, const size_t a_Size);

void D_RBSWriteInt8(D_RBlockStream_t* const a_Stream, const int8_t a_Val);
void D_RBSWriteInt16(D_RBlockStream_t* const a_Stream, const int16_t a_Val);
void D_RBSWriteInt32(D_RBlockStream_t* const a_Stream, const int32_t a_Val);
void D_RBSWriteInt64(D_RBlockStream_t* const a_Stream, const int64_t a_Val);
void D_RBSWriteUInt8(D_RBlockStream_t* const a_Stream, const uint8_t a_Val);
void D_RBSWriteUInt16(D_RBlockStream_t* const a_Stream, const uint16_t a_Val);
void D_RBSWriteUInt32(D_RBlockStream_t* const a_Stream, const uint32_t a_Val);
void D_RBSWriteUInt64(D_RBlockStream_t* const a_Stream, const uint64_t a_Val);

void D_RBSWriteString(D_RBlockStream_t* const a_Stream, const char* const a_Val);
void D_RBSWritePointer(D_RBlockStream_t* const a_Stream, const void* const a_Ptr);

size_t D_RBSReadChunk(D_RBlockStream_t* const a_Stream, void* const a_Data, const size_t a_Size);

int8_t D_RBSReadInt8(D_RBlockStream_t* const a_Stream);
int16_t D_RBSReadInt16(D_RBlockStream_t* const a_Stream);
int32_t D_RBSReadInt32(D_RBlockStream_t* const a_Stream);
int64_t D_RBSReadInt64(D_RBlockStream_t* const a_Stream);
uint8_t D_RBSReadUInt8(D_RBlockStream_t* const a_Stream);
uint16_t D_RBSReadUInt16(D_RBlockStream_t* const a_Stream);
uint32_t D_RBSReadUInt32(D_RBlockStream_t* const a_Stream);
uint64_t D_RBSReadUInt64(D_RBlockStream_t* const a_Stream);

size_t D_RBSReadString(D_RBlockStream_t* const a_Stream, char* const a_Out, const size_t a_OutSize);
uint64_t D_RBSReadPointer(D_RBlockStream_t* const a_Stream);

/**************************
*** GENERIC BYTE STREAM ***
**************************/

/* GenericByteStream -- A Generic Byte Stream (abstract) */
class GenericByteStream_c
{
	private:
		bool p_IsUnicode;						// UTF-16/32 Stream
		bool p_IsSwapped;						// Byte Swapped 16/32 Stream
		char p_MBBuf[5];						// Multi-byte buffer
		size_t p_MBLeft;						// Characters left in buffer
		
	public:
		GenericByteStream_c();
		~GenericByteStream_c();
		
		/* Abstracted */
		virtual bool Seekable(void) = 0;
		virtual bool EndOfStream(void) = 0;
		virtual uint64_t Tell(void) = 0;
		virtual uint64_t Seek(const uint64_t a_NewPos, const bool a_AtEnd = false) = 0;
		virtual size_t ReadChunk(void* const a_Data, const size_t a_Size) = 0;
		virtual size_t WriteChunk(const void* const a_Data, const size_t a_Size) = 0;
		
		/* Reading */
		int8_t ReadInt8(void);
		int16_t ReadInt16(void);
		int32_t ReadInt32(void);
		int64_t ReadInt64(void);
		uint8_t ReadUInt8(void);
		uint16_t ReadUInt16(void);
		uint32_t ReadUInt32(void);
		uint64_t ReadUInt64(void);
		
		/* Writing */
		void WriteInt8(const int8_t a_Value);
		void WriteInt16(const int16_t a_Value);
		void WriteInt32(const int32_t a_Value);
		void WriteInt64(const int64_t a_Value);
		void WriteUInt8(const uint8_t a_Value);
		void WriteUInt16(const uint16_t a_Value);
		void WriteUInt32(const uint32_t a_Value);
		void WriteUInt64(const uint64_t a_Value);
		
		/* Little Endian */
		int16_t ReadLittleInt16(void);
		int32_t ReadLittleInt32(void);
		int64_t ReadLittleInt64(void);
		uint16_t ReadLittleUInt16(void);
		uint32_t ReadLittleUInt32(void);
		uint64_t ReadLittleUInt64(void);
		
		void WriteLittleInt16(const int16_t a_Value);
		void WriteLittleInt32(const int32_t a_Value);
		void WriteLittleInt64(const int64_t a_Value);
		void WriteLittleUInt16(const uint16_t a_Value);
		void WriteLittleUInt32(const uint32_t a_Value);
		void WriteLittleUInt64(const uint64_t a_Value);
		
		/* Big Endian */
		int16_t ReadBigInt16(void);
		int32_t ReadBigInt32(void);
		int64_t ReadBigInt64(void);
		uint16_t ReadBigUInt16(void);
		uint32_t ReadBigUInt32(void);
		uint64_t ReadBigUInt64(void);
		
		void WriteBigInt16(const int16_t a_Value);
		void WriteBigInt32(const int32_t a_Value);
		void WriteBigInt64(const int64_t a_Value);
		void WriteBigUInt16(const uint16_t a_Value);
		void WriteBigUInt32(const uint32_t a_Value);
		void WriteBigUInt64(const uint64_t a_Value);
		
		/* Strings */
		size_t ReadString(char* const a_Buf, const size_t a_Size);
		size_t ReadStringN(char* const a_Buf, const size_t a_Size);
		void WriteString(const char* const a_Buf);
		void WriteStringN(const char* const a_Buf, const size_t a_Size);
		
		/* UTF-8 */
		bool CheckUnicode(void);
		
		// UTF-8 Layer
		char ReadChar(void);
		size_t ReadLine(char* const a_Buf, const size_t a_Size);
		
		void WriteChar(const char a_Value);
		
		// UTF-16/32 Layer
		wchar_t ReadWChar(void);
		size_t ReadWLine(wchar_t* const a_Buf, const size_t a_Size);
		
		void WriteWChar(const wchar_t a_Value);
};

/********************
*** RBSTREAM CORE ***
********************/

/* RBAddress_c -- Address to send blocks */
class RBAddress_c
{
	public:
		enum AddressType_e
		{
			AT_NOTINIT,							// Not Initialized
			AT_LOCAL,							// Local Address
			AT_NET,								// Net Address
		};
		
	private:
		AddressType_e p_Type;					// Type of address
		
	public:
		RBAddress_c();
		~RBAddress_c();
		
		void SetFrom(const RBAddress_c& a_B);
		
		bool CompareAddress(const RBAddress_c& a_B) const;
		static bool CompareAddress(const RBAddress_c& a_A, const RBAddress_c& a_B);
		
		void ChooseLocal(void);
};

/* RBStream_c -- Base class for derived block streams */
class RBStream_c : public GenericByteStream_c
{
	friend class RBAddress_c;
		
	protected:
		/* Info */
		void* p_Data;							// Private Data
		uint32_t p_Flags;						// Flags (if ever needed)
	
		/* Current Block */
		char p_BlkHeader[5];					// Block identifier
		uint8_t* p_BlkData;						// Data
		size_t p_BlkSize;						// Block Size
		size_t p_BlkBufferSize;					// Size of buffer
		size_t p_ReadOff;						// Read Offset
		bool p_Marked;							// Marked?
		
	public:
		/* Constructors and Destructors */
		RBStream_c();
		~RBStream_c();
		
		/* Abstracted */
		bool Seekable(void);
		bool EndOfStream(void);
		uint64_t Tell(void);
		uint64_t Seek(const uint64_t a_NewPos, const bool a_AtEnd = false);
		size_t ReadChunk(void* const a_Data, const size_t a_Size);
		size_t WriteChunk(const void* const a_Data, const size_t a_Size);
		
		/* RBS Stuff */
		bool BlockBase(const char* const a_Header);
		bool BlockRename(const char* const a_Header);
		bool HeaderCopy(char* const a_Dest);
		size_t BlockSize(void);
		uint8_t* BlockData(void);
		
		/* Handled by sub classes */
		virtual bool BlockPlay(RBAddress_c* const a_Address = NULL) = 0;
		virtual bool BlockRecord(RBAddress_c* const a_Address = NULL) = 0;
		virtual bool BlockFlush(void) = 0;
};

/* RBLoopBackStream_c -- Loopback Stream (Master) */
class RBLoopBackStream_c : public RBStream_c
{
	private:
		struct RBLoopBackHold_s
		{
			uint64_t FlushID;							// Current flush ID
			char Header[5];								// Header
			uint8_t* Data;								// Data
			size_t Size;								// Size
		};
		
		uint64_t p_FlushID;							// FlushID
		RBLoopBackHold_s** p_Q;						// Blocks in Q
		size_t p_SizeQ;								// Size of Q
	
	public:
		RBLoopBackStream_c();
		~RBLoopBackStream_c();
		
		/* Handled by sub classes */
		bool BlockPlay(RBAddress_c* const a_Address = NULL);
		bool BlockRecord(RBAddress_c* const a_Address = NULL);
		bool BlockFlush(void);
};

/* RBWLEStream_c -- Streamer that wraps WLEntryStream_c (Master) */
class WLEntryStream_c;
class RBWLEStream_c : public RBStream_c
{
	private:
		WLEntryStream_c* p_WLStream;				// Based WL Stream
		
	public:
		RBWLEStream_c(WLEntryStream_c* const a_Stream);
		~RBWLEStream_c();
		
		/* Handled by sub classes */
		bool BlockPlay(RBAddress_c* const a_Address = NULL);
		bool BlockRecord(RBAddress_c* const a_Address = NULL);
		bool BlockFlush(void);
};

/* RBPerfectStream_c -- Perfect Stream (Slave) */
class RBPerfectStream_c : public RBStream_c
{
	private:
		friend class RBStream_c;
		friend class GenericByteStream_c;
		
		/* RBPerfectKey_s -- Perfect Key */
		struct RBPerfectKey_s
		{
			uint32_t Key[4];
			RBAddress_c RemHost;				// Remote Host
			uint64_t NextReadNum;				// Next packet to read
			uint64_t NextWriteNum;				// Next packet to write

			uint32_t CreateTime;				// Time when key was created
			uint32_t LastTime;					// Last activity time
			uint32_t ExpireTime;				// Time when key expires
			bool ExpireLessThan;				// Expire time is in the past
			bool Revoke;						// Revoke key
		};
		
		/* RBPerfectHold_s -- Holding store */
		struct RBPerfectHold_s
		{
			/* Standard */
			char Header[5];						// Header
			uint8_t* Data;						// Data
			size_t Size;						// Size
	
			/* Perfect Networking */
			uint32_t ReTryCount;				// Retransmit count
			uint64_t PacketNum;					// Packet Number
			uint32_t CheckSum;					// Simplified Checksum
			uint32_t ClockTime;					// Time packet was stored
			uint32_t AutoRackTime;				// time to retransmit
			bool RackTimeIsLess;				// Less than
			bool ReTransmit;					// Retransmit block
			bool BlockAck;						// Block acknowledged
			RBAddress_c RemHost;				// Remote Host
	
			// Packet Key
			uint32_t Key[4];					// Key Holder
		};
		
		RBStream_c* p_WrapStream;				// Stream to wrap
		bool p_InFlush;							// In the middle of a flush

		RBPerfectHold_s** p_ReadQ;				// Blocks to read Q
		size_t p_SizeReadQ;						// Size of read Q

		RBPerfectHold_s** p_WriteQ;				// Blocks to read Q
		size_t p_SizeWriteQ;					// Size of read Q

		RBPerfectKey_s** p_Keys;				// Packet keys
		size_t p_NumKeys;						// Number of keys

		bool p_Debug;							// Debug it
		
		RBPerfectKey_s* IntFindKey(const uint32_t* const a_InKey, RBAddress_c* const a_Address);
		
	public:
		RBPerfectStream_c(RBStream_c* const a_WrapStream);
		~RBPerfectStream_c();
		
		/* Handled by sub classes */
		bool BlockPlay(RBAddress_c* const a_Address = NULL);
		bool BlockRecord(RBAddress_c* const a_Address = NULL);
		bool BlockFlush(void);
};

#endif /* __D_BLOCK_H__ */

