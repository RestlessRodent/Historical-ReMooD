// -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-
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
// Copyright (C) 1993-1996 by id Software, Inc.
// Copyright (C) 1998-2000 by DooM Legacy Team.
// Copyright (C) 2008-2012 GhostlyDeath (ghostlydeath@gmail.com)
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
// DESCRIPTION:
//      Preparation of data for rendering,
//      generation of lookups, caching, retrieval by name.

#include "doomdef.h"
#include "g_game.h"
#include "i_video.h"
#include "r_local.h"
#include "r_sky.h"
#include "p_local.h"
#include "r_data.h"
#include "w_wad.h"
#include "z_zone.h"
#include "p_setup.h"			//levelflats
#include "v_video.h"			//pLoaclPalette

#ifdef _WIN32
#include "malloc.h"
#endif

int firstflat, lastflat, numflats;
int firstpatch, lastpatch, numpatches;
int firstspritelump, lastspritelump, numspritelumps;

// textures
int numtextures = 0;			// total number of textures found,

// size of following tables

texture_t** textures = NULL;

// can easily repeat along sidedefs using
// a simple mask

// needed for pre rendering
fixed_t* spritewidth;
fixed_t* spriteoffset;
fixed_t* spritetopoffset;
fixed_t* spriteheight;			//SoM

lighttable_t* colormaps = NULL;

//faB: for debugging/info purpose
int flatmemory;
int spritememory;
int texturememory;

//
// MAPTEXTURE_T CACHING
// When a texture is first needed,
//  it counts the number of composite columns
//  required in the texture and allocates space
//  for a column directory and any new columns.
// The directory will simply point inside other patches
//  if there is only one patch in a given column,
//  but any columns with multiple patches
//  will have new column_ts generated.
//

//
// R_DrawColumnInCache
// Clip and draw a column
//  from a patch into a cached post.
//

void R_DrawColumnInCache(column_t* patch, uint8_t* cache, int originy, int cacheheight)
{
	int count;
	int position;
	uint8_t* source;
	uint8_t* dest;
	int i;
	
	dest = (uint8_t*)cache;	// + 3;
	
	while (patch->topdelta != 0xff)
	{
		source = (uint8_t*)patch + 3;
		count = patch->length;
		position = originy + patch->topdelta;
		
		if (position < 0)
		{
			count += position;
			position = 0;
		}
		
		if (position + count > cacheheight)
			count = cacheheight - position;
			
		if (count > 0)
			memcpy(cache + position, source, count);
			
		patch = (column_t*) ((uint8_t*)patch + patch->length + 4);
	}
}

//
// Empty the texture cache (used for load wad at runtime)
//
void R_FlushTextureCache(void)
{
	int i;
	
	if (numtextures > 0)
		for (i = 0; i < numtextures; i++)
		{
			//if (texturecache[i])
			//	Z_Free(texturecache[i]);
		}
}

/* R_PatchInfo_t -- Information on a patch */
typedef struct R_PatchInfo_s
{
	/* Per WAD */
	char Name[9];								// Name of patch
	uint32_t Hash;								// Hash of patch
	const WL_WADEntry_t* Entry;					// Entry in WAD
	V_Image_t* Image;							// Image for this patch
} R_PatchInfo_t;

/* R_TextureHolder_t -- Holds texture stuff */
typedef struct R_TextureHolder_s
{
	/* Walls and Floors */
	R_PatchInfo_t* PatchInfos;					// Patch information (complex)
	size_t NumPatches;							// Number of PNAMES in wad
	char* RealPatchBuf;							// Real patch buffer
	char** Patches;								// List of names to numbers
	
	texture_t* Textures;						// Texture Data
	uint32_t* TextureOffsets;					// Texture offsets
	size_t NumTextures;							// Number of textures
	
	/* Sprites */
	size_t NumSpriteInfo;						// Number of sprites
	R_SpriteInfoEx_t* SpriteInfo;				// Sprite information
} R_TextureHolder_t;

static R_PatchInfo_t* l_PatchList = NULL;		// List of patches
static size_t l_NumPatchList;					// Number of patches in list
static Z_HashTable_t* l_TextureHashes = NULL;

/* RS_TexturePDRemove() -- Remove loaded texture data */
static void RS_TexturePDRemove(const struct WL_WADFile_s* a_WAD)
{
}

/* RS_GetMarkers() -- Get markers in WAD */
static bool_t RS_GetMarkers(const WL_WADFile_t* const a_WAD, const char* const a_Start, const char* const a_End, const WL_WADEntry_t** const a_SOut, const WL_WADEntry_t** const a_EOut)
{
	const WL_WADEntry_t* SFound;
	const WL_WADEntry_t* EFound;
	
	/* Check */
	if (!a_WAD || !a_Start || !a_End || !a_SOut || !a_EOut)
		return false;
	
	/* Find entries in wad */
	// Start first
	SFound = WL_FindEntry(a_WAD, 0, a_Start);
	EFound = WL_FindEntry(a_WAD, 0, a_End);
	
	// Not found?
	if (!SFound || !EFound)
		return false;
	
	/* Compare index */
	if (EFound->Index <= SFound->Index)
		return false;	// End before start?
	
	/* Move start marker up */
	SFound = SFound->NextEntry;
	
	// No more?
	if (!SFound)
		return false;
	
	/* Make sure end was not hit */
	if (SFound->Index == EFound->Index)
		return false;
	
	/* Found it */
	*a_SOut = SFound;
	*a_EOut = EFound;
	return true;
}

/* RS_TexturePDCreate() -- Create texture data for WADs */
// Not only does this do textures, it does sprites also.
// I thought about putting sprite loaders in a separate function but putting it
// here would simplify things. Sprites are needed for the server anyway, at
// least for scripting and pickups.
static bool_t RS_TexturePDCreate(const struct WL_WADFile_s* const a_WAD, const uint32_t a_Key, void** const a_DataPtr, size_t* const a_SizePtr, WL_RemoveFunc_t* const a_RemoveFuncPtr)
{
#define TEMPNAME 9
	R_TextureHolder_t* Holder;
	const WL_WADEntry_t* Entry;
	const WL_WADEntry_t* MStart;
	const WL_WADEntry_t* MEnd;
	WL_EntryStream_t* Stream;
	size_t i, j, k, p, z;
	char c;
	bool_t Continue;
	char TempStr[TEMPNAME];
	size_t BaseOffset, ThisCount;
	
	/* Check */
	if (!a_WAD || !a_DataPtr || !a_SizePtr)
		return false;
	
	/* Create Data Holder */
	*a_SizePtr = sizeof(R_TextureHolder_t);
	Holder = *a_DataPtr = Z_Malloc(*a_SizePtr, PU_STATIC, NULL);
	
	/* Map PNAMES to actual names */
	// Normally when textures are loaded, the texture information references
	// patches by their ID. However, when cases where the patch is replaced,
	// this cannot be done. Messing with the PNAMES order causes the wrong
	// patches to be used for textures and whatnot. There is also the case for
	// the dynamic WAD loading code. A texture could be replaced and there could
	// be patches in later WADs. Although this changes the behavior of how
	// textures are handled, it simplifies things when textures are added and
	// removed of which they may have illegal references.
	
	// Locate PNAMES
	Entry = WL_FindEntry(a_WAD, 0, "PNAMES");
	
	// Found it?
	if (Entry)
	{
		// Open stream
		Stream = WL_StreamOpen(Entry);
		
		// Opened?
		if (Stream)
		{
			// Read number of patches
			Holder->NumPatches = WL_StreamReadLittleUInt32(Stream);
			
			// Allocate array name of patches
			Holder->RealPatchBuf = Z_Malloc(sizeof(char) * 9 * Holder->NumPatches, PU_STATIC, NULL);
			Holder->Patches = Z_Malloc(sizeof(*Holder->Patches) * Holder->NumPatches, PU_STATIC, NULL);
			
			// Read in patch names
			for (i = 0; i < Holder->NumPatches; i++)
			{
				// Allocate in sub buffer
				Holder->Patches[i] = &Holder->RealPatchBuf[i * 9];
				
				// Read in lump data
				for (Continue = true, j = 0; j < 8; j++)
				{
					// Always read character
					c = WL_StreamReadChar(Stream);
					
					// No longer continue?
					if (!c)
						Continue = false;
					
					// Add to buffer
					if (Continue)
						Holder->Patches[i][j] = c;
				}
			}
			
			// Close stream
			WL_StreamClose(Entry);
		}
	}
	
	/* Process TEXTUREx Lumps */
	// Allow from TEXTURE1..9
	for (BaseOffset = 0, i = 0; i <= 9; i++)
	{
		// Build name of lump
		snprintf(TempStr, TEMPNAME, "TEXTURE%u", (unsigned int)i);
		
		// Attempt to locate it
		if (!(Entry = WL_FindEntry(a_WAD, 0, TempStr)))
			continue;	// Not found, ignore
		
		// Attempt to open stream
		if (!(Stream = WL_StreamOpen(Entry)))
			continue;	// Failed to open
		
		// Read texture count (and add)
		ThisCount = WL_StreamReadLittleUInt32(Stream);
		
		// Allocate offset table
		Holder->TextureOffsets = Z_Malloc(sizeof(*Holder->TextureOffsets) * ThisCount, PU_STATIC, NULL);
		
		// Re-allocate texture table
		Z_ResizeArray((void**)&Holder->Textures, sizeof(*Holder->Textures), Holder->NumTextures, Holder->NumTextures + ThisCount);
		Holder->NumTextures += ThisCount;
		
		// Read offset table
		for (j = 0; j < ThisCount; j++)
			Holder->TextureOffsets[j] = WL_StreamReadLittleUInt32(Stream);
		
		// Read in texture data
		for (z = BaseOffset, j = 0; j < ThisCount; j++, z++)
		{
			// Seek to offset in stream
			WL_StreamSeek(Stream, Holder->TextureOffsets[j], false);
			
			// Set internal order
			Holder->Textures[z].InternalOrder = j;
			
			// Read texture name
			for (Continue = true, k = 0; k < 8; k++)
			{
				// Always read character
				c = WL_StreamReadChar(Stream);
				
				// No longer continue?
				if (!c)
					Continue = false;
				
				// Add to buffer
				if (Continue)
					Holder->Textures[z].name[k] = c;
			}
			
			// Ignore "masked", unused
			WL_StreamReadLittleUInt32(Stream);
			
			// Read the size of the texture
			Holder->Textures[z].width = WL_StreamReadLittleInt16(Stream);
			Holder->Textures[z].height = WL_StreamReadLittleInt16(Stream);
			
			// Fixed-ize the dimensions
			Holder->Textures[z].XWidth = ((fixed_t)Holder->Textures[z].width) << FRACBITS;
			Holder->Textures[z].XHeight = ((fixed_t)Holder->Textures[z].height) << FRACBITS;
			
			// Create mask
			Holder->Textures[z].WidthMask = Holder->Textures[z].width - 1;
			
			// Non-power of two?
			for (k = Holder->Textures[z].width; k; k >>= 1)
				// Bit 1 and other bit set?
					// This works because: 1000f -> 100f -> 10f -> 1f
					//                     1010f -> 101t
				if ((k & 1) && (k & (~1)))
				{
					Holder->Textures[z].NonPowerTwo;
					break;
				}
			
			// Ignore "columndirectory", unused
			WL_StreamReadLittleUInt32(Stream);
			
			// Read patch count
			Holder->Textures[z].patchcount = WL_StreamReadLittleUInt16(Stream);
			
			// Allocate patch data
			if (Holder->Textures[z].patchcount)
				Holder->Textures[z].patches = Z_Malloc(sizeof(*Holder->Textures[z].patches) * Holder->Textures[z].patchcount, PU_STATIC, NULL);
			
			// Read in patches
			for (p = 0; p < Holder->Textures[z].patchcount; p++)
			{
				// Read offsets
				Holder->Textures[z].patches[p].originx = WL_StreamReadLittleInt16(Stream);
				Holder->Textures[z].patches[p].originy = WL_StreamReadLittleInt16(Stream);
				
				// Read patch ID
				Holder->Textures[z].patches[p].patch = WL_StreamReadLittleUInt16(Stream);
				
				// Ignore stepdir and colormap
				WL_StreamReadLittleUInt16(Stream);
				WL_StreamReadLittleUInt16(Stream);
				
				// Translate patch ID to name
				k = Holder->Textures[z].patches[p].patch;
				
				// Copy name over
				if (k >= 0 && k < Holder->NumPatches)
					strncpy(Holder->Textures[z].patches[p].PatchName, Holder->Patches[k], 8);
			}
		}
		
		// Close stream
		WL_StreamClose(Stream);
		
		// Free offset table (no longer needed)
		if (Holder->TextureOffsets)
			Z_Free(Holder->TextureOffsets);
		Holder->TextureOffsets = NULL;
		
		// Increase base
		BaseOffset += Holder->NumTextures;
	}
	
	/* Free up space used by PNAMES */
	// Since the texture code pre-referenced patch names instead of numbers, the
	// PNAMES data is no longer required.
	if (Holder->RealPatchBuf)
		Z_Free(Holder->RealPatchBuf);
	Holder->RealPatchBuf = NULL;
	
	if (Holder->Patches)
		Z_Free(Holder->Patches);
	Holder->Patches = NULL;
	
	/* Process Flats */
	// Flats can now be used as textures, however there are differences.
	// Flats will appear as standard 64x64 textures, they will also be of lower
	// priority compared to textures. If a flat is called SOMEPIC and a texture
	// is called SOMEPIC, whichever one is chosen is based on the order. If the
	// renderer requested a wall texture, the texture would be returned, but if
	// a flat was requested the true flat will be returned. This will result in
	// two textures having the same name but different IDs. But in the other
	// case, if no texture or flat exists by said name, it will fall through and
	// use another flat or texture by that same name (i.e. asking for a wall
	// called SOMEFLAT but there is no wall texture by that name, it will return
	// the texture for that flat instead).
	Entry = MStart = MEnd = NULL;
	
	// Attempt flat marker location
	if (!RS_GetMarkers(a_WAD, "F_START", "F_END", &MStart, &MEnd))
		if (!RS_GetMarkers(a_WAD, "FF_START", "F_END", &MStart, &MEnd))
			if (!RS_GetMarkers(a_WAD, "F_START", "FF_END", &MStart, &MEnd))
				RS_GetMarkers(a_WAD, "FF_START", "FF_END", &MStart, &MEnd);
	
	// Read list of flats
	if (MStart && MEnd)
	{
		// Get number of flats
		BaseOffset = Holder->NumTextures;
		ThisCount = MEnd->Index - MStart->Index;
		
		// Reallocate texture array
		Z_ResizeArray((void**)&Holder->Textures, sizeof(*Holder->Textures), Holder->NumTextures, Holder->NumTextures + ThisCount);
		Holder->NumTextures += ThisCount;
		
		// Read in textures by lumps
		for (i = BaseOffset, Entry = MStart; Entry && Entry != MEnd; i++, Entry = Entry->NextEntry)
		{
			// Copy name
			strncat(Holder->Textures[i].name, Entry->Name, 8);
			
			// Set internal order to what i is
			Holder->Textures[i].InternalOrder = i;
			
			// Flats are always 64x64
			Holder->Textures[i].width = 64;
			Holder->Textures[i].height = 64;
			Holder->Textures[i].XWidth = 64 << FRACBITS;
			Holder->Textures[i].XHeight = 64 << FRACBITS;
			
			// Mask
			Holder->Textures[i].WidthMask = Holder->Textures[i].width - 1;
			
			// Mark as flat
			Holder->Textures[i].IsFlat = true;
			
			// Set entry to this
			Holder->Textures[i].FlatEntry = Entry;
		}
	}
	
	/* Process sprites */
	Entry = MStart = MEnd = NULL;
	
	// Attempt flat marker location
	if (!RS_GetMarkers(a_WAD, "S_START", "S_END", &MStart, &MEnd))
		if (!RS_GetMarkers(a_WAD, "SS_START", "S_END", &MStart, &MEnd))
			if (!RS_GetMarkers(a_WAD, "S_START", "SS_END", &MStart, &MEnd))
				RS_GetMarkers(a_WAD, "SS_START", "SS_END", &MStart, &MEnd);
	
	// Read list of sprites
	if (MStart && MEnd)
	{
		// Create sprite info array
		Holder->NumSpriteInfo = MEnd->Index - MStart->Index;
		Holder->SpriteInfo = Z_Malloc(sizeof(*Holder->SpriteInfo) * Holder->NumSpriteInfo, PU_STATIC, (void**)&Holder->SpriteInfo);
		
		// Read in sprites by lumps
		for (i = 0, Entry = MStart; Entry && Entry != MEnd; Entry = Entry->NextEntry)
		{
			// Check name length (nnnnFR or nnnnFRFR)
			z = strlen(Entry->Name);
			
			if (z != 6 && z != 8)
				continue;
			
			// Check for legal state/frame
			c = toupper(Entry->Name[4]);
			if (c < 'A' || c > '`')
				continue;
			
			c = toupper(Entry->Name[5]);
			if (!((c >= '0' && c <= '9') || (c >= 'A' && c <= 'G')))
				continue;
			
			// Check second
			if (z == 8)
			{
				c = toupper(Entry->Name[6]);
				if (c < 'A' || c > '`')
					continue;
				
				c = toupper(Entry->Name[7]);
				if (!((c >= '0' && c <= '9') || (c >= 'A' && c <= 'G')))
					continue;
			}
			
			// Copy name (to uppercase) and create code
			Holder->SpriteInfo[i].Code = 0;
			for (j = 0; j < 4; j++)
			{
				c = toupper(Entry->Name[j]);
				
				Holder->SpriteInfo[i].Name[j] = c;
				Holder->SpriteInfo[i].Code |= ((uint32_t)c) << (8 * j);
			}
			
			// Determine frame and rotation
			for (j = 0; j < 2; j++)
			{
				// Frame requires no real checking
				c = toupper(Entry->Name[4 + (j * 2)]);
				Holder->SpriteInfo[i].Frame[j] = c - 'A';
				
				// Rotation does however
				c = toupper(Entry->Name[5 + (j * 2)]);
				if (c >= '0' || c <= '9')
					Holder->SpriteInfo[i].Rotation[j] = c - '0';
				else
					Holder->SpriteInfo[i].Rotation[j] = (c - 'A') + 10;
			}
			
			// Mark double?
			Holder->SpriteInfo[i].Double = false;
			if (z == 8)
				Holder->SpriteInfo[i].Double = true;
			
			// Place entry
			Holder->SpriteInfo[i].Entry = Entry;
			
			// Increment i
			i++;
		}
	}
	
	/* Success */
	return true;
#undef TEMPNAME
}

/* R_TCFInput_t -- Input on the table */
typedef struct R_TCFInput_s
{
	bool_t AnyMode;								// Find anything
	bool_t CheckFlat;							// Check for flat
	const char* Name;							// Name to check
} R_TCFInput_t;

/* RS_TextureCompareFunc() -- Checks if two textures match */
// A = R_TCFInput_t*
// B = uintptr_t
static bool_t RS_TextureCompareFunc(void* const a_A, void* const a_B)
{
	R_TCFInput_t* TCF;
	uintptr_t TextID;
	
	/* Check */
	if (!a_A || (((uintptr_t)a_B) - 1) >= numtextures)
		return false;
	
	/* Set */
	TCF = (R_TCFInput_t*)a_A;
	TextID = ((uintptr_t)a_B) - 1;
	
	/* Check again */
	if (!TCF->Name)
		return false;
	
	/* Not any mode and flat is not matched to flat */
	// We want a specific texture/flat, but we got something else
	if (!TCF->AnyMode && TCF->CheckFlat != textures[TextID]->IsFlat)
		return false;
	
	/* Compare the name of the texture */
	if (strcasecmp(TCF->Name, textures[TextID]->name) == 0)
		return true;
	
	/* No match */
	return false;
}

/* RS_HashUpTextures() -- Hash the textures */
static bool_t RS_HashUpTextures(void)
{
	size_t i;
	uint32_t Hash;
	
	/* Check */
	if (!numtextures || !textures)
		return false;
	
	/* Create table */
	l_TextureHashes = Z_HashCreateTable(RS_TextureCompareFunc);
	
	/* Run through textures creating hashes for them */
	for (i = 0; i < numtextures; i++)
	{
		// Create hash of name
		Hash = Z_Hash(textures[i]->name);
		
		// Add to table with table offset increased by 1
		Z_HashAddEntry(l_TextureHashes, Hash, (uintptr_t)(i + 1));
	}
	
	/* Success is with us always */
	return true;
}

/* RS_TextureOrderChange() -- Order changed (rebuild composite) */
static bool_t RS_TextureOrderChange(const bool_t a_Pushed, const struct WL_WADFile_s* const a_WAD)
{
	R_TCFInput_t TCF;
	const WL_WADFile_t* Rover;
	uint32_t OrderMul, Hash;
	R_TextureHolder_t* Holder;
	bool_t First;
	size_t HolderSize, i, j, k, z, PutWall, PutFloor, b, n, y, Best;
	texture_t** TempArray;
	uintptr_t FoundID;
	const WL_WADEntry_t* Entry;
	const WL_WADEntry_t* MStart;
	const WL_WADEntry_t* MEnd;
	R_SpriteInfoEx_t* ThisInfo;
	spritedef_t* ThisDef;
	spriteframe_t* ThisFrame;
	uint32_t Code;
	
	/* Clear texture count */
	// Free pointer array
	if (textures)
		Z_Free(textures);
	textures = NULL;
	
	// Clear count
	numtextures = 0;
	
	// Free patch composite
	if (l_PatchList)
		Z_Free(l_PatchList);
	l_PatchList = NULL;
	
	// Clear count
	l_NumPatchList = 0;
	
	// Delete hash table
	if (l_TextureHashes)
		Z_HashDeleteTable(l_TextureHashes);
	l_TextureHashes = NULL;
	
	/* Go through WAD files in reverse order */
	// Why reverse? So later textures take precedence, when a texture is to be
	// slapped onto the pointer list, see if a texture of the same name already
	// exists. If it does, do not add it.
	OrderMul = 900;		// 900 is hopefully enough (to retain order of textures)
	for (First = true, Rover = WL_IterateVWAD(NULL, false); Rover; Rover = WL_IterateVWAD(Rover, false), OrderMul--)
	{
		// Obtain texture info
		Holder = WL_GetPrivateData(Rover, WLDK_TEXTURES, &HolderSize);
		
		// Nothing here?
		if (!Holder)
			continue;
		
		// No textures in this WAD?
		if (!Holder->NumTextures)
			continue;
		
		// If this is the first, then there is no need to slow down texture loading
		// as everything is exactly how it is.
		if (First)
		{
			// Create pointer array
			Z_ResizeArray((void**)&textures, sizeof(*textures), numtextures, numtextures + Holder->NumTextures);
			numtextures = Holder->NumTextures;
			
			// Copy pointers
			for (i = 0; i < numtextures; i++)
			{
				textures[i] = &Holder->Textures[i];
				
				// Set combined order
				textures[i]->CombinedOrder = (OrderMul << 16) + textures[i]->InternalOrder;
			}
			
			// Unset first
			First = false;
		}
		
		// This is not the first WAD with textures, so when a texture is
		// iterated check to see if it already exists in the global texture
		// table. If it does not appear in it, add it to the back.
		else
		{
			// Does a hash table need to be created?
			if (!l_TextureHashes)
				RS_HashUpTextures();
			
			// Clear marks on this WAD's textures and see if they deserve marking
			for (y = 0, i = 0; i < Holder->NumTextures; i++)
			{
				// Clear mark
				Holder->Textures[i].Marked = false;
				
				// See if the texture already exists
				memset(&TCF, 0, sizeof(TCF));
				TCF.Name = Holder->Textures[i].name;
				TCF.CheckFlat = Holder->Textures[i].IsFlat;
				Hash = Z_Hash(Holder->Textures[i].name);
	
				FoundID = (uintptr_t)Z_HashFindEntry(l_TextureHashes, Hash, &TCF, false);
				
				// If it was found, then do not mark it
				if (FoundID)
					continue;
				
				// Found so mark it and increase total
				Holder->Textures[i].Marked = true;
				y++;
			}
			
			// Found new textures to append?
			if (y)
			{
				// Destroy invalid hash table
				if (l_TextureHashes)
					Z_HashDeleteTable(l_TextureHashes);
				l_TextureHashes = NULL;
				
				// Increase array size by the count and remember the old count
				b = numtextures;
				Z_ResizeArray((void**)&textures, sizeof(*textures), numtextures, numtextures + y);
				numtextures += y;
				
				// Go through list again for marked ones
				for (i = 0; i < Holder->NumTextures; i++)
					if (Holder->Textures[i].Marked)
					{
						// Reference then set order
						textures[b] = &Holder->Textures[i];
						textures[b]->CombinedOrder = (OrderMul << 16) + textures[b]->InternalOrder;
						
						// Increment b, the base write index
						b++;
					}
			}
		}
	}
	
	/* Sort texture table */
	// Destroy hash table (it will be made invalid)
	if (l_TextureHashes)
		Z_HashDeleteTable(l_TextureHashes);
	l_TextureHashes = NULL;
	
	// Sort by wall textures and floor textures (walls first, then floors)
	PutWall = 0;
	PutFloor = numtextures - 1;
	TempArray = Z_Malloc(sizeof(*TempArray) * numtextures, PU_STATIC, NULL);
	
	for (i = 0; i < numtextures; i++)
		if (textures[i]->IsFlat)
			TempArray[PutFloor--] = textures[i];
		else
			TempArray[PutWall++] = textures[i];
	
	// Sort sub zones by combinedID. (TempArray back into textures)
	for (y = 0, z = 0; z < 2; z++)
	{
		// Walls
		if (!z)
		{
			b = 0;
			n = PutWall;
		}
		
		// Floors
		else
		{
			b = PutWall;
			n = b + (numtextures - PutFloor);
		}
		
		// Sort through (memory: 2, selection sort)
		for (i = b; i < n; i++)
		{
			// Initial best
			for (Best = b; Best < n; Best++)
				if (TempArray[Best])
					break;
			
			// Find lowest
			for (j = b; j < n; j++)
				if (TempArray[j])
					if (TempArray[j]->CombinedOrder < TempArray[Best]->CombinedOrder)
						Best = j;
			
			// Place best here
			textures[y++] = TempArray[Best];
			
			// Wipe out best value
			TempArray[Best] = NULL;
		}
	}
	
	// Temporary array is not needed anymore
	Z_Free(TempArray);
	
	// Recreate hash table (to make it fully valid)
	RS_HashUpTextures();
	
	/* Finalize textures */
	for (i = 0; i < numtextures; i++)
	{
		// Translate to self initially
		textures[i]->Translation = i;
		
		// Reference patches
		for (j = 0; j < textures[i]->patchcount; j++)
		{
			// Find entry for this texture
			textures[i]->patches[j].Entry = WL_FindEntry(NULL, 0, textures[i]->patches[j].PatchName);
			
			// No match?
			if (devparm)
				if (!textures[i]->patches[j].Entry)
					CONL_PrintF("Patch without entry %s.\n", textures[i]->patches[j].PatchName);
				
			
				
			// Hash name
			Hash = Z_Hash(textures[i]->patches[j].PatchName);
			
			// Look through existing list for match
			for (b = 0; b < l_NumPatchList; b++)
				if (Hash == l_PatchList[i].Hash)
					if (strcasecmp(textures[i]->patches[j].PatchName, l_PatchList[i].Name) == 0)
						break;
			
			// Not found, append to back
			if (b >= l_NumPatchList)
			{
				// Resize array
				Z_ResizeArray((void**)&l_PatchList, sizeof(*l_PatchList), l_NumPatchList, l_NumPatchList + 1);
				l_NumPatchList += 1;
				
				// Fill in info
				strncpy(l_PatchList[b].Name, textures[i]->patches[j].PatchName, 8);
				l_PatchList[b].Hash = Hash;
				
				// Find the real patch picture that belongs to this
				l_PatchList[b].Entry = textures[i]->patches[j].Entry;
			}
			
			// Reference it from b (if appended, b is the last)
			textures[i]->patches[j].PatchListRef = b;
		}
	}
	
	/* Merge sprite information */
	// This turns R_SpriteInfoEx_t -> spritedef_t + spriteframe_t
	for (Rover = WL_IterateVWAD(NULL, true); Rover; Rover = WL_IterateVWAD(Rover, true))
	{
		// Obtain texture info
		Holder = WL_GetPrivateData(Rover, WLDK_TEXTURES, &HolderSize);
		
		// No sprites?
		if (!Holder->NumSpriteInfo)
			continue;
		
		// Go through every sprite
		for (i = 0; i < Holder->NumSpriteInfo; i++)
		{
			// Base pointers
			ThisInfo = &Holder->SpriteInfo[i];
			ThisDef = NULL;
			
			// Try to find sprite def with the same definition
			for (j = 0; j < g_NumExSprites; j++)
				if (g_ExSprites[j].Code == ThisInfo->Code)
				{
					ThisDef = &g_ExSprites[j];
					break;
				}
			
			// Was it not found? append to end
			if (!ThisDef)
			{
				// Resize and slap at the end
				Z_ResizeArray((void**)&g_ExSprites, sizeof(*g_ExSprites), g_NumExSprites, g_NumExSprites + 1);
				
				// Get the very end
				ThisDef = &g_ExSprites[g_NumExSprites++];
				
				// Place code here
				ThisDef->Code = ThisInfo->Code;
				
				// Create frame stuff
				ThisDef->numframes = MAXEXSPRITEFRAMES;
				ThisDef->spriteframes = Z_Malloc(sizeof(*ThisDef->spriteframes) * ThisDef->numframes, PU_STATIC, NULL);
			}
			
			// Go through each single or double set
			for (j = 0; j < 1 + (ThisInfo->Double ? 1 : 0); j++)
			{
				// Frame out of bounds?
				if (ThisInfo->Frame[j] < 0 || ThisInfo->Frame[j] >= MAXEXSPRITEFRAMES)
					continue;
				
				// Get frame ref
				ThisFrame = &ThisDef->spriteframes[ThisInfo->Frame[j]];
				
				// All rotations?
				if (ThisInfo->Rotation[j] == 0)
				{
					// Set all to this
					for (k = 0; k < 16; k++)
					{
						ThisFrame->ExAngles[k] = ThisInfo;
						ThisFrame->ExFlip[k] = !!j;
					}
				}
				
				// A single rotation
				else
				{
					// Only set a single rotation
					k = ThisInfo->Rotation[j] - 1;
					
					// Check bounds
					if (k >= 0 && k < 16)
					{
						ThisFrame->ExAngles[k] = ThisInfo;
						ThisFrame->ExFlip[k] = !!j;
					}
				}
			}
		}
	}
	
	// Map to sprites[numsprites] (which requires spritenum_t compat)
	// Once info.[ch] is shed away, this can be removed.
	// The states in info.[ch] uses sprites along with sprnames to determine
	// the sprite to draw.
	if (sprites)
		Z_Free(sprites);
	sprites = NULL;
	
	numsprites = NUMSPRITES;
	sprites = Z_Malloc(sizeof(*sprites) * numsprites, PU_STATIC, (void**)&sprites);
	
	for (i = 0; i < numsprites; i++)
	{
		// Get code name for this sprite
		Code = 0;
		for (j = 0; j < 4; j++)
			Code |= ((uint32_t)toupper(sprnames[i][j])) << (8 * j);
		
		// Find matching code
		for (j = 0; j < g_NumExSprites; j++)
		{
			char ba[5] = {0, 0, 0, 0, 0};
			char bb[5] = {0, 0, 0, 0, 0};
			
			memcpy(ba, &g_ExSprites[j].Code, 4);
			memcpy(bb, &Code, 4);
			
			if (g_ExSprites[j].Code == Code)
			{
				sprites[i] = g_ExSprites[j];
				break;
			}
		}
	}
		
	/* Success */
	return true;
}

/* R_InitExSpriteInfo() -- Initialize extended sprite info */
void R_InitExSpriteInfo(R_SpriteInfoEx_t* const a_Info)
{
	int32_t w, h, xo, yo;
	
	/* Check */
	if (!a_Info)
		return;
	
	/* Already initted? */
	if (a_Info->Init)
		return;
	
	/* Get image from entry */
	a_Info->Image = V_ImageLoadE(a_Info->Entry);
	
	// Get properties
	if (a_Info->Image)
	{
		// Get from image
		w = h = xo = yo = 0;
		V_ImageSizePos(a_Info->Image, &w, &h, &xo, &yo);
		
		// Set to info
		a_Info->Width = w << FRACBITS;
		a_Info->Height = h << FRACBITS;
		a_Info->Offset = xo << FRACBITS;
		a_Info->TopOffset = yo << FRACBITS;
	}
	
	/* Set to initialized */
	a_Info->Init = true;
}

/* R_CheckTextureNumForName() -- Find texture by name */
int R_CheckTextureNumForName(char* name)
{
	R_TCFInput_t TCF;
	uintptr_t Ref;
	uint32_t Hash;
	
	/* Check */
	if (!name)
		return 0;
	
	/* Create TCF and hash name */
	memset(&TCF, 0, sizeof(TCF));
	TCF.Name = name;
	Hash = Z_Hash(name);
	
	/* Find in table */
	Ref = (uintptr_t)Z_HashFindEntry(l_TextureHashes, Hash, &TCF, false);
	
	// Not found?
	if (!Ref)
	{
		// Just look for anything then
		TCF.AnyMode = true;
		
		Ref = (uintptr_t)Z_HashFindEntry(l_TextureHashes, Hash, &TCF, false);
		
		// Still not found (return ASHWALL/AASTINKY)
		if (!Ref)
			return 0;
	}
	
	/* Found it! */
	return Ref - 1;
}

/* R_TextureNumForName() -- Find texture by name */
int R_TextureNumForName(char* name)
{
	/* Just call the texture check function */
	return R_CheckTextureNumForName(name);
}

/* R_GetFlatNumForName() -- Get flat by name */
int R_GetFlatNumForName(char* name)
{
	R_TCFInput_t TCF;
	uintptr_t Ref;
	uint32_t Hash;
	
	/* Check */
	if (!name)
		return 0;
	
	/* Create TCF and hash name */
	memset(&TCF, 0, sizeof(TCF));
	TCF.Name = name;
	TCF.CheckFlat = true;
	Hash = Z_Hash(name);
	
	/* Find in table */
	Ref = (uintptr_t)Z_HashFindEntry(l_TextureHashes, Hash, &TCF, false);
	
	// Not found?
	if (!Ref)
	{
		// Just look for anything then
		TCF.AnyMode = true;
		
		Ref = (uintptr_t)Z_HashFindEntry(l_TextureHashes, Hash, &TCF, false);
		
		// Still not found (return ASHWALL/AASTINKY)
		if (!Ref)
			return 0;
	}
	
	/* Found it! */
	return Ref - 1;
}

/* R_GetFlat() -- Get data for flat */
uint8_t* R_GetFlat(int flatlumpnum)
{
	texture_t* Texture;
	V_Image_t* Image;
	uint8_t* FlatData;
	size_t Size, p;
	uint32_t w, h;
	R_PatchInfo_t* PInfo;
	
	/* Check */
	if (flatlumpnum < 0 || flatlumpnum >= numtextures)
		return NULL;
	
	/* Reference texture */
	Texture = textures[flatlumpnum];
	w = 64;//Texture->width;
	h = 64;//Texture->height;
	
	/* Check if cache exists */
	if (Texture->FlatCache)
		return Texture->FlatCache;
	
	// Debug
	if (devparm)
		CONL_PrintF("R_GetFlat: Generating \"%s\".\n", Texture->name);
	
	/* Create cache */
	Texture->FlatCache = Z_Malloc(w * h, PU_STATIC, (void**)&Texture->FlatCache);
	
	/* If texture is a flat, return patch_t of it */
	if (Texture->IsFlat)
	{
		// Image already exists?
		if (Texture->FlatImage)
			Image = Texture->FlatImage;
		
		// Load image
		else
			Image = Texture->FlatImage = V_ImageLoadE(Texture->FlatEntry);
		
		// Draw into buffer
		if (Image)
			V_ImageDrawScaledIntoBuffer(VEX_IGNOREOFFSETS, Image, 0, 0, 0, 0, 1 << (FRACBITS), 1 << (FRACBITS), NULL, Texture->FlatCache, w, w, h, 1 << FRACBITS, 1 << FRACBITS, 1.0, 1.0);
	}
	
	/* Otherwise it is a normal texture with patches */
	else
	{
		// For every patch in the texture, draw
		for (p = 0; p < Texture->patchcount; p++)
		{
			// Obtain info
			PInfo = &l_PatchList[Texture->patches[p].PatchListRef];
		
			// Image needs loading?
			if (!PInfo->Image)
				PInfo->Image = V_ImageLoadE(PInfo->Entry);
		
			// Draw into buffer
			V_ImageDrawScaledIntoBuffer(VEX_IGNOREOFFSETS, PInfo->Image, Texture->patches[p].originx, Texture->patches[p].originy, 0, 0, 1 << (FRACBITS), 1 << (FRACBITS), NULL, Texture->FlatCache, w, w, h, 1 << FRACBITS, 1 << FRACBITS, 1.0, 1.0);
		}
	}
	
	/* Change to static */
	Z_ChangeTag(Texture->FlatCache, PU_CACHE);
	
	/* Return flat cache */
	return Texture->FlatCache;
}

/* R_GenerateTexture() -- Generates a texture (loads from cache) */
uint8_t* R_GenerateTexture(int texnum)
{
	uint8_t* Buffer;
	uint8_t* dd;
	uint32_t w, h, x, y, z;
	size_t p, c, PatchSize;
	texture_t* Texture;
	V_Image_t* Image;
	patch_t* PatchT;
	R_PatchInfo_t* PInfo;
	size_t Size;
	
	/* Check */
	if (texnum < 0 || texnum >= numtextures)
		return NULL;
	
	/* Quick info */
	Texture = textures[texnum];
	w = Texture->width;
	h = Texture->height;
	
	/* Wipe old data */
	// Clear offsets
	if (Texture->ColumnOffs)
		Z_Free(Texture->ColumnOffs);
	Texture->ColumnOffs = NULL;
	
	// Clear composite
	if (Texture->Composite)
	{
		for (c = 0; c < Texture->width; c++)
			if (Texture->Composite[c])
				Z_Free(Texture->Composite[c]);
		Z_Free(Texture->Composite);
	}
	Texture->Composite = NULL;
	
	// Clear cache size
	Texture->CacheSize = 0;
	
	/* Allocate buffer based on size */
	Buffer = Z_Malloc(sizeof(*Buffer) * (w * h), PU_STATIC, NULL);
	
	/* Drawing a flat? */
	if (Texture->IsFlat)
	{
		// Image already exists?
		if (Texture->FlatImage)
			Image = Texture->FlatImage;
		
		// Load image
		else
			Image = Texture->FlatImage = V_ImageLoadE(Texture->FlatEntry);
		
		// Draw into buffer
		if (Image)
			V_ImageDrawScaledIntoBuffer(VEX_IGNOREOFFSETS, Image, 0, 0, 0, 0, 1 << (FRACBITS), 1 << (FRACBITS), NULL, Buffer, w, w, h, 1 << FRACBITS, 1 << FRACBITS, 1.0, 1.0);
	}
	
	/* Drawing a wall? */
	else
	{
		// For every patch in the texture, draw
		for (p = 0; p < Texture->patchcount; p++)
		{
			// Obtain info
			PInfo = &l_PatchList[Texture->patches[p].PatchListRef];
		
			// Image needs loading?
			if (!PInfo->Image)
				PInfo->Image = V_ImageLoadE(PInfo->Entry);
		
			// Draw into buffer
			V_ImageDrawScaledIntoBuffer(VEX_IGNOREOFFSETS, PInfo->Image, Texture->patches[p].originx, Texture->patches[p].originy, 0, 0, 1 << (FRACBITS), 1 << (FRACBITS), NULL, Buffer, w, w, h, 1 << FRACBITS, 1 << FRACBITS, 1.0, 1.0);
		}
	}
	
	/* Create column data based on picture */
	Texture->ColumnOffs = Z_Malloc(sizeof(*Texture->ColumnOffs) * w, PU_STATIC, NULL);
	Texture->Cache = Z_Malloc((w * h) + (6 * w), PU_CACHE, NULL);
	
	// Copy in data
	dd = Texture->Cache;
	z = 0;
	for (x = 0; x < w; x++)
	{
		// Offset here
		Texture->ColumnOffs[x] = dd - Texture->Cache;
		
		// Place posts
		*(dd++) = 0;	// offset
		*(dd++) = h;	// size
		*(dd++) = 0;	// junk
		
		for (y = 0; y < h; y++)
			//*(dd++) = x + y + w + h;
			*(dd++) = Buffer[(w * y) + x];
		
		*(dd++) = 0;	// junk
		*(dd++) = 0xFFU;	// end
	}
	
	/* No longer need the buffer */
	Z_Free(Buffer);
	
	/* Return cache */
	return Texture->Cache;
}

/* R_GetColumn() -- Get column data from a texture */
uint8_t* R_GetColumn(int tex, size_t col)
{
	/* Check */
	if (tex < 0 || tex >= numtextures)
		return NULL;
	
	/* Needs generation? */
	if (!textures[tex]->Cache)
	{
		if (devparm)
			CONL_PrintF("R_GetColumn: Generating \"%s\".\n", textures[tex]->name);
		R_GenerateTexture(tex);
	}
	
	/* Return texture column */
	//return ((uint8_t*)textures[tex]->Composite
	return ((uint8_t*)textures[tex]->Cache) + (textures[tex]->ColumnOffs[col & textures[tex]->WidthMask] + 3);
}

/* R_LoadTextures() -- Loads texture data information */
// GhostlyDeath <December 14, 2011> -- Stubby
void R_LoadTextures(void)
{
	/* Register order callbacks and generators */
	// Generators first
	if (!WL_RegisterPDC(WLDK_TEXTURES, WLDPO_TEXTURES, RS_TexturePDCreate, RS_TexturePDRemove))
		I_Error("R_LoadTextures: Failed to register PDC.\n");
	
	// Order callback
	if (!WL_RegisterOCCB(RS_TextureOrderChange, WLDCO_TEXTURES))
		I_Error("R_LoadTextures: Failed to register OCCB.\n");
}

/* R_CheckNumForNameList() -- Find flat */
WadIndex_t R_CheckNumForNameList(char* name, lumplist_t* list, int listsize)
{
	int i;
	WadIndex_t lump = INVALIDLUMP;
	
	for (i = listsize - 1; i > -1; i--)
	{
		lump = W_CheckNumForNamePwadPtr(name, list[i].WadFile, list[i].firstlump);
		
		// TODO -- GhostlyDeath <June 21, 2009> -- Check this validity here
		if ((lump - W_LumpsSoFar(list[i].WadFile)) > (list[i].firstlump + list[i].numlumps) || lump == INVALIDLUMP)
			continue;
		else
			return lump;
	}
	
	return INVALIDLUMP;
}

lumplist_t* colormaplumps;
int numcolormaplumps;

void R_InitExtraColormaps()
{
	WadFile_t* scout = NULL;
	uint32_t i;
	int Start = 0;
	int End = 0;
	int clump = 0;
	
	for (i = 0; i < W_NumWadFiles(); i++, clump = 0)
	{
		scout = W_GetWadForNum(i);
		
		Start = W_CheckNumForNamePwadPtr("C_START", scout, clump);
		
		if (Start == -1)
			continue;
			
		End = W_CheckNumForNamePwadPtr("C_END", scout, clump);
		
		if (End == -1)
			continue;
			
		// TODO - GhostlyDeath: freedoom.wad map01 crashed with "Error: R_ColormapNumForName: Cannot find colormap lump MFADEMAP"
		colormaplumps = (lumplist_t*) realloc(colormaplumps, sizeof(lumplist_t) * (numcolormaplumps + 1));
		colormaplumps[numcolormaplumps].WadFile = scout;
		colormaplumps[numcolormaplumps].firstlump = Start + 1;
		colormaplumps[numcolormaplumps].numlumps = End - (Start + 1);
		numcolormaplumps++;
	}
	
	/*int       startnum;
	   int       endnum;
	   int       cfile;
	   int       clump;
	
	   numcolormaplumps = 0;
	   colormaplumps = NULL;
	   cfile = clump = 0;
	
	   for(;cfile < numwadfiles;cfile ++, clump = 0)
	   {
	   startnum = W_CheckNumForNamePwad("C_START", cfile, clump);
	   if(startnum == -1)
	   continue;
	
	   endnum = W_CheckNumForNamePwad("C_END", cfile, clump);
	
	   if(endnum == -1)
	   I_Error("R_InitColormaps: C_START without C_END\n");
	
	   if((startnum >> 16) != (endnum >> 16))
	   I_Error("R_InitColormaps: C_START and C_END in different wad files!\n");
	
	   colormaplumps = (lumplist_t *)realloc(colormaplumps, sizeof(lumplist_t) * (numcolormaplumps + 1));
	   //colormaplumps[numcolormaplumps].wadfile = startnum >> 16;
	   colormaplumps[numcolormaplumps].firstlump = (startnum&0xFFFF) + 1;
	   colormaplumps[numcolormaplumps].numlumps = endnum - (startnum + 1);
	   numcolormaplumps++;
	   } */
}

lumplist_t* flats;
int numflatlists;

extern int numwadfiles;

/* R_InitFlats() -- Initialize flats */
void R_InitFlats()
{
	// GhostlyDeath <December 14, 2011> -- The flat code will be unified with
	// the texture code. This will allow textures as flats and vice versa. It
	// also helps unify it for future OpenGLization.
}

size_t g_SpritesBufferSize = 0;	// GhostlyDeath <July 24, 2011> -- Unlimited sprites! not really

/* R_SetSpriteLumpCount() -- Sets a nice size for the sprite buffer */
void R_SetSpriteLumpCount(const size_t a_Count)
{
	size_t BufferMul;
	
	/* Obtain buffer multiple */
	BufferMul = (a_Count / NUMSPRITEBUMPS) + 1;
	
	// No resize needed?
	if (BufferMul <= g_SpritesBufferSize)
		return;
		
	/* Resize all arrays */
	Z_ResizeArray(&spritewidth, sizeof(*spritewidth) * NUMSPRITEBUMPS, g_SpritesBufferSize, BufferMul);
	Z_ResizeArray(&spriteoffset, sizeof(*spriteoffset) * NUMSPRITEBUMPS, g_SpritesBufferSize, BufferMul);
	Z_ResizeArray(&spritetopoffset, sizeof(*spritetopoffset) * NUMSPRITEBUMPS, g_SpritesBufferSize, BufferMul);
	Z_ResizeArray(&spriteheight, sizeof(*spriteheight) * NUMSPRITEBUMPS, g_SpritesBufferSize, BufferMul);
	
	/* Recount total */
	g_SpritesBufferSize = BufferMul;
}

//
// R_InitSpriteLumps
// Finds the width and hoffset of all sprites in the wad,
//  so the sprite does not need to be cached completely
//  just for having the header info ready during rendering.
//

//
//   allocate sprite lookup tables
//
void R_InitSpriteLumps(void)
{
	// the original Doom used to set numspritelumps from S_END-S_START+1
	
	//Fab:FIXME: find a better solution for adding new sprites dynamically
	numspritelumps = 0;
	
	spritewidth = spriteoffset = spritetopoffset = spriteheight = NULL;
	
	R_SetSpriteLumpCount(1);	// Just to bump it a bit
}

void R_InitExtraColormaps();
void R_ClearColormaps();

/* RS_ColormapOCCB() -- Handles colormaps */
static bool_t RS_ColormapOCCB(const bool_t a_Pushed, const struct WL_WADFile_s* const a_WAD)
{
	WL_WADEntry_t* Entry;
	
	/* Load base colormap */
	Entry = WL_FindEntry(NULL, 0, "COLORMAP");
	
	// Found?
	if (Entry)
	{
		// Free old colormap
		if (colormaps)
			Z_Free(colormaps);
		
		// Allocate new space
		colormaps = Z_Malloc(Entry->Size, PU_STATIC, NULL);
		WL_ReadData(Entry, 0, colormaps, Entry->Size);
	}
	
	/* Boom Stuff */
	R_ClearColormaps();
	R_InitExtraColormaps();
	
	/* Success! */
	return true;
}

/* R_InitColormaps() -- Loads all the colormaps */
// GhostlyDeath <December 14, 2011> -- WLized
void R_InitColormaps(void)
{
	/* Register OCCB */
	if (!WL_RegisterOCCB(RS_ColormapOCCB, 100))
		I_Error("R_InitColormaps: Failed to register OCCB.");
}

int foundcolormaps[MAXCOLORMAPS];

//SoM: Clears out extra colormaps between levels.
void R_ClearColormaps()
{
	int i;
	
	num_extra_colormaps = 0;
	for (i = 0; i < MAXCOLORMAPS; i++)
		foundcolormaps[i] = -1;
	memset(extra_colormaps, 0, sizeof(extra_colormaps));
}

int R_ColormapNumForName(char* name)
{
	int lump, i;
	
	if (num_extra_colormaps == MAXCOLORMAPS)
		I_Error("R_ColormapNumForName: Too many colormaps!\n");
		
	lump = R_CheckNumForNameList(name, colormaplumps, numcolormaplumps);
	if (lump == -1)
		lump = W_GetNumForName(name);	//I_Error("R_ColormapNumForName: Cannot find colormap lump %s\n", name);
		
	for (i = 0; i < num_extra_colormaps; i++)
		if (lump == foundcolormaps[i])
			return i;
			
	foundcolormaps[num_extra_colormaps] = lump;
	
	// aligned on 8 bit for asm code
	extra_colormaps[num_extra_colormaps].colormap = Z_MallocAlign(W_LumpLength(lump), PU_LEVEL, 0, 8);
	W_ReadLump(lump, extra_colormaps[num_extra_colormaps].colormap);
	
	// SoM: Added, we set all params of the colormap to normal because there
	// is no real way to tell how GL should handle a colormap lump anyway..
	extra_colormaps[num_extra_colormaps].maskcolor = 0xffff;
	extra_colormaps[num_extra_colormaps].fadecolor = 0x0;
	extra_colormaps[num_extra_colormaps].maskamt = 0x0;
	extra_colormaps[num_extra_colormaps].fadestart = 0;
	extra_colormaps[num_extra_colormaps].fadeend = 33;
	extra_colormaps[num_extra_colormaps].fog = 0;
	
	num_extra_colormaps++;
	return num_extra_colormaps - 1;
}

// SoM:
//
// R_CreateColormap
// This is a more GL friendly way of doing colormaps: Specify colormap
// data in a special linedef's texture areas and use that to generate
// custom colormaps at runtime. NOTE: For GL mode, we only need to color
// data and not the colormap data.
double deltas[256][3], map[256][3];

unsigned char NearestColor(unsigned char r, unsigned char g, unsigned char b);
int RoundUp(double number);

int R_CreateColormap(char* p1, char* p2, char* p3)
{
	double cmaskr, cmaskg, cmaskb, cdestr, cdestg, cdestb;
	double r, g, b;
	double cbrightness;
	double maskamt = 0, othermask = 0;
	int mask;
	int i, p;
	char* colormap_p;
	unsigned int cr, cg, cb;
	unsigned int maskcolor, fadecolor;
	unsigned int fadestart = 0, fadeend = 33, fadedist = 33;
	int fog = 0;
	int mapnum = num_extra_colormaps;
	
#define HEX2INT(x) (x >= '0' && x <= '9' ? x - '0' : x >= 'a' && x <= 'f' ? x - 'a' + 10 : x >= 'A' && x <= 'F' ? x - 'A' + 10 : 0)
	if (p1[0] == '#')
	{
		cr = cmaskr = ((HEX2INT(p1[1]) * 16) + HEX2INT(p1[2]));
		cg = cmaskg = ((HEX2INT(p1[3]) * 16) + HEX2INT(p1[4]));
		cb = cmaskb = ((HEX2INT(p1[5]) * 16) + HEX2INT(p1[6]));
		// Create a rough approximation of the color (a 16 bit color)
		maskcolor = ((cb) >> 3) + (((cg) >> 2) << 5) + (((cr) >> 3) << 11);
		if (p1[7] >= 'a' && p1[7] <= 'z')
			mask = (p1[7] - 'a');
		else if (p1[7] >= 'A' && p1[7] <= 'Z')
			mask = (p1[7] - 'A');
		else
			mask = 24;
			
		maskamt = (double)mask / (double)24;
		
		othermask = 1 - maskamt;
		maskamt /= 0xff;
		cmaskr *= maskamt;
		cmaskg *= maskamt;
		cmaskb *= maskamt;
	}
	else
	{
		cmaskr = 0xff;
		cmaskg = 0xff;
		cmaskb = 0xff;
		maskamt = 0;
		maskcolor = ((0xff) >> 3) + (((0xff) >> 2) << 5) + (((0xff) >> 3) << 11);
	}
	
#define NUMFROMCHAR(c)  (c >= '0' && c <= '9' ? c - '0' : 0)
	if (p2[0] == '#')
	{
		// SoM: Get parameters like, fadestart, fadeend, and the fogflag...
		fadestart = NUMFROMCHAR(p2[3]) + (NUMFROMCHAR(p2[2]) * 10);
		fadeend = NUMFROMCHAR(p2[5]) + (NUMFROMCHAR(p2[4]) * 10);
		if (fadestart > 32 || fadestart < 0)
			fadestart = 0;
		if (fadeend > 33 || fadeend < 1)
			fadeend = 33;
		fadedist = fadeend - fadestart;
		fog = NUMFROMCHAR(p2[1]) ? 1 : 0;
	}
#undef getnum
	
	if (p3[0] == '#')
	{
		cdestr = cr = ((HEX2INT(p3[1]) * 16) + HEX2INT(p3[2]));
		cdestg = cg = ((HEX2INT(p3[3]) * 16) + HEX2INT(p3[4]));
		cdestb = cb = ((HEX2INT(p3[5]) * 16) + HEX2INT(p3[6]));
		fadecolor = (((cb) >> 3) + (((cg) >> 2) << 5) + (((cr) >> 3) << 11));
	}
	else
	{
		cdestr = 0;
		cdestg = 0;
		cdestb = 0;
		fadecolor = 0;
	}
#undef HEX2INT
	
	for (i = 0; i < num_extra_colormaps; i++)
	{
		if (foundcolormaps[i] != -1)
			continue;
		if (maskcolor == extra_colormaps[i].maskcolor &&
		        fadecolor == extra_colormaps[i].fadecolor &&
		        maskamt == extra_colormaps[i].maskamt &&
		        fadestart == extra_colormaps[i].fadestart && fadeend == extra_colormaps[i].fadeend && fog == extra_colormaps[i].fog)
			return i;
	}
	
	if (num_extra_colormaps == MAXCOLORMAPS)
		I_Error("R_CreateColormap: Too many colormaps!\n");
	num_extra_colormaps++;
	
	for (i = 0; i < 256; i++)
	{
		r = pLocalPalette[i].s.red;
		g = pLocalPalette[i].s.green;
		b = pLocalPalette[i].s.blue;
		cbrightness = sqrt((r * r) + (g * g) + (b * b));
		
		map[i][0] = (cbrightness * cmaskr) + (r * othermask);
		if (map[i][0] > 255.0)
			map[i][0] = 255.0;
		deltas[i][0] = (map[i][0] - cdestr) / (double)fadedist;
		
		map[i][1] = (cbrightness * cmaskg) + (g * othermask);
		if (map[i][1] > 255.0)
			map[i][1] = 255.0;
		deltas[i][1] = (map[i][1] - cdestg) / (double)fadedist;
		
		map[i][2] = (cbrightness * cmaskb) + (b * othermask);
		if (map[i][2] > 255.0)
			map[i][2] = 255.0;
		deltas[i][2] = (map[i][2] - cdestb) / (double)fadedist;
	}
	
	foundcolormaps[mapnum] = -1;
	
	// aligned on 8 bit for asm code
	extra_colormaps[mapnum].colormap = NULL;
	extra_colormaps[mapnum].maskcolor = maskcolor;
	extra_colormaps[mapnum].fadecolor = fadecolor;
	extra_colormaps[mapnum].maskamt = maskamt;
	extra_colormaps[mapnum].fadestart = fadestart;
	extra_colormaps[mapnum].fadeend = fadeend;
	extra_colormaps[mapnum].fog = fog;
	
#define ABS2(x) ((x) < 0 ? -(x) : (x))
	extra_colormaps[mapnum].colormap = colormap_p = Z_MallocAlign((256 * 34) + 10, PU_LEVEL, 0, 16);	// Aligning on 16 bits, NOT 8, keeps it from crashing! SSNTails 12-13-2002
	
	for (p = 0; p < 34; p++)
	{
		for (i = 0; i < 256; i++)
		{
			*colormap_p = NearestColor(RoundUp(map[i][0]), RoundUp(map[i][1]), RoundUp(map[i][2]));
			colormap_p++;
			
			if ((unsigned int)p < fadestart)
				continue;
				
			if (ABS2(map[i][0] - cdestr) > ABS2(deltas[i][0]))
				map[i][0] -= deltas[i][0];
			else
				map[i][0] = cdestr;
				
			if (ABS2(map[i][1] - cdestg) > ABS2(deltas[i][1]))
				map[i][1] -= deltas[i][1];
			else
				map[i][1] = cdestg;
				
			if (ABS2(map[i][2] - cdestb) > ABS2(deltas[i][1]))
				map[i][2] -= deltas[i][2];
			else
				map[i][2] = cdestb;
		}
	}
#undef ABS2
	
	return mapnum;
}

//Thanks to quake2 source!
//utils3/qdata/images.c
unsigned char NearestColor(unsigned char r, unsigned char g, unsigned char b)
{
	int dr, dg, db;
	int distortion;
	int bestdistortion = 256 * 256 * 4;
	int bestcolor = 0;
	int i;
	
	for (i = 0; i < 256; i++)
	{
		dr = r - pLocalPalette[i].s.red;
		dg = g - pLocalPalette[i].s.green;
		db = b - pLocalPalette[i].s.blue;
		distortion = dr * dr + dg * dg + db * db;
		if (distortion < bestdistortion)
		{
		
			if (!distortion)
				return i;
				
			bestdistortion = distortion;
			bestcolor = i;
		}
	}
	
	return bestcolor;
}

// Rounds off floating numbers and checks for 0 - 255 bounds
int RoundUp(double number)
{
	if (number > 255.0)
		return 255.0;
	if (number < 0)
		return 0;
		
	if ((int)number <= (int)(number - 0.5))
		return (int)number + 1;
		
	return (int)number;
}

char* R_ColormapNameForNum(int num)
{
	if (num == -1)
		return "NONE";
		
	if (num < 0 || num > MAXCOLORMAPS)
		I_Error("R_ColormapNameForNum: num is invalid!\n");
		
	if (foundcolormaps[num] == -1)
		return "INLEVEL";
		
	return (W_GetWadForNum(foundcolormaps[num] >> 16))->Index[foundcolormaps[num] & 0xffff].Name;
	
	//return wadfiles[foundcolormaps[num] >> 16]->lumpinfo[foundcolormaps[num] & 0xffff].name;
}

//
// R_InitData
// Locates all the lumps
//  that will be used by all views
// Must be called after W_Init.
//
void R_InitData(void)
{
	CONL_PrintF("\nInitFlats...");
	R_InitFlats();
	
	CONL_PrintF("\nInitSprites...\n");
	R_InitSpriteLumps();
	R_InitSprites(sprnames);
	
	CONL_PrintF("\nInitColormaps...\n");
	R_InitColormaps();
}

//SoM: REmoved R_FlatNumForName

//
// R_PrecacheLevel
// Preloads all relevant graphics for the level.
//

// BP: rules : no extern in c !!!
//     slution put a new function in p_setup.c or put this in global (not recommended)
// SoM: Ok.. Here it goes. This function is in p_setup.c and caches the flats.
int P_PrecacheLevelFlats();

void R_PrecacheLevel(void)
{
//  char*               flatpresent; //SoM: 4/18/2000: No longer used
	char* texturepresent;
	char* spritepresent;
	
	int i;
	int j;
	int k;
	int lump;
	
	thinker_t* th;
	spriteframe_t* sf;
	
	//int numgenerated;  //faB:debug
	
	if (demoplayback)
		return;
		
	// do not flush the memory, Z_Malloc twice with same user
	// will cause error in Z_CheckHeap(), 19991022 by Kin
	
	// Precache flats.
	/*flatpresent = alloca(numflats);
	   memset (flatpresent,0,numflats);
	
	   // Check for used flats
	   for (i=0 ; i<numsectors ; i++)
	   {
	   #ifdef PARANOIA
	   if( sectors[i].floorpic<0 || sectors[i].floorpic>numflats )
	   I_Error("sectors[%d].floorpic=%d out of range [0..%d]\n",i,sectors[i].floorpic,numflats);
	   if( sectors[i].ceilingpic<0 || sectors[i].ceilingpic>numflats )
	   I_Error("sectors[%d].ceilingpic=%d out of range [0..%d]\n",i,sectors[i].ceilingpic,numflats);
	   #endif
	   flatpresent[sectors[i].floorpic] = 1;
	   flatpresent[sectors[i].ceilingpic] = 1;
	   }
	
	   flatmemory = 0;
	
	   for (i=0 ; i<numflats ; i++)
	   {
	   if (flatpresent[i])
	   {
	   lump = firstflat + i;
	   if(devparm)
	   flatmemory += W_LumpLength(lump);
	   R_GetFlat (lump);
	   //            W_CacheLumpNum(lump, PU_CACHE);
	   }
	   } */
	flatmemory = P_PrecacheLevelFlats();
	
	//
	// Precache textures.
	//
	// no need to precache all software textures in 3D mode
	// (note they are still used with the reference software view)
	texturepresent = Z_Malloc(numflats, PU_STATIC, NULL);
	memset(texturepresent, 0, numtextures);
	
	for (i = 0; i < numsides; i++)
	{
		//Hurdler: huh, a potential bug here????
		if (sides[i].toptexture < numtextures)
			texturepresent[sides[i].toptexture] = 1;
		if (sides[i].midtexture < numtextures)
			texturepresent[sides[i].midtexture] = 1;
		if (sides[i].bottomtexture < numtextures)
			texturepresent[sides[i].bottomtexture] = 1;
	}
	
	// Sky texture is always present.
	// Note that F_SKY1 is the name used to
	//  indicate a sky floor/ceiling as a flat,
	//  while the sky texture is stored like
	//  a wall texture, with an episode dependend
	//  name.
	texturepresent[skytexture] = 1;
	
	//if (devparm)
	//    CONL_PrintF("Generating textures..\n");
	
	texturememory = 0;
	for (i = 0; i < numtextures; i++)
	{
		if (!texturepresent[i])
			continue;
			
		//texture = textures[i];
		//if (texturecache[i] == NULL)
		//	R_GenerateTexture(i);
		//numgenerated++;
		
		// note: pre-caching individual patches that compose textures became
		//       obsolete since we now cache entire composite textures
		
		//for (j=0 ; j<texture->patchcount ; j++)
		//{
		//    lump = texture->patches[j].patch;
		//    texturememory += W_LumpLength(lump);
		//    W_CacheLumpNum(lump , PU_CACHE);
		//}
	}
	//CONL_PrintF ("total mem for %d textures: %d k\n",numgenerated,texturememory>>10);
	
	//
	// Precache sprites.
	//
	spritepresent = Z_Malloc(numsprites, PU_STATIC, NULL);
	memset(spritepresent, 0, numsprites);
	
	for (th = thinkercap.next; th != &thinkercap; th = th->next)
	{
		if (th->function.acp1 == (actionf_p1) P_MobjThinker)
			spritepresent[((mobj_t*)th)->sprite] = 1;
	}
	
	spritememory = 0;
	for (i = 0; i < numsprites; i++)
	{
		if (!spritepresent[i])
			continue;
			
		for (j = 0; j < sprites[i].numframes; j++)
		{
			sf = &sprites[i].spriteframes[j];
			for (k = 0; k < 8; k++)
			{
				//Fab: see R_InitSprites for more about lumppat,lumpid
				lump = /*firstspritelump + */ sf->lumppat[k];
				if (devparm)
					spritememory += W_LumpLength(lump);
				W_CachePatchNum(lump, PU_CACHE);
			}
		}
	}
	
	//FIXME: this is no more correct with glide render mode
	if (devparm)
	{
		CONL_PrintF("Precache level done:\n"
		            "flatmemory:    %ld k\n" "texturememory: %ld k\n" "spritememory:  %ld k\n", flatmemory >> 10, texturememory >> 10, spritememory >> 10);
	}
}

