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
// Copyright (C) 2008-2011 GhostlyDeath (ghostlydeath@gmail.com)
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
uint32_t** texturecolumnofs;	// column offset lookup table for each texture
uint8_t** texturecache;			// graphics data for each generated full-size texture
int* texturewidthmask;			// texture width is a power of 2, so it

// can easily repeat along sidedefs using
// a simple mask
fixed_t* textureheight;			// needed for texture pegging

int* flattranslation;			// for global animation
int* texturetranslation;

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

uint8_t* R_GenerateTexture(int texnum);

//
// R_GetColumn
//
uint8_t* R_GetColumn(int tex, size_t col)
{
	uint8_t* data;
	
	col &= texturewidthmask[tex];
	data = texturecache[tex];
	
	if (data == NULL)
	{
		if (devparm)
			CONL_PrintF("R_GetColumn: Texture %i does not exist, generating!\n", tex);
		data = R_GenerateTexture(tex);
	}
	
	return data + texturecolumnofs[tex][col];
}

//
// R_GenerateTexture
//
//   Allocate space for full size texture, either single patch or 'composite'
//   Build the full textures from patches.
//   The texture caching system is a little more hungry of memory, but has
//   been simplified for the sake of highcolor, dynamic ligthing, & speed.
//
//   This is not optimised, but it's supposed to be executed only once
//   per level, when enough memory is available.
//
uint8_t* R_GenerateTexture(int texnum)
{
	uint8_t* block;
	uint8_t* blocktex;
	texture_t* texture;
	texpatch_t* patch;
	patch_t* realpatch;
	int x;
	int x1;
	int x2;
	int i, j;
	column_t* patchcol;
	uint32_t* colofs;
	size_t blocksize;
	
	texture = textures[texnum];
	
	// allocate texture column offset lookup
	
	// single-patch textures can have holes in it and may be used on
	// 2sided lines so they need to be kept in 'packed' format
	if (texture->patchcount == 1)
	{
		patch = texture->patches;
		blocksize = W_LumpLength(patch->patch);
#if 1
		realpatch = W_CacheLumpNum(patch->patch, PU_CACHE);
		
		block = Z_Malloc(blocksize, PU_STATIC,	// will change tag at end of this function
		                 &texturecache[texnum]);
		memset(block, 0, blocksize);
		memcpy(block, realpatch, blocksize);
#else
		// FIXME: this version don't put the user z_block
		texturecache[texnum] = block = W_CacheLumpNum(patch->patch, PU_STATIC, &texturecache[texnum]);
#endif
		//CONL_PrintF ("R_GenTex SINGLE %.8s size: %d\n",texture->name,blocksize);
		
		texturememory += blocksize;
		
		// GhostlyDeath -- Wait a minute! No wonder texturecache[n] is always NULL!
		texturecache[texnum] = block;
		
		// use the patch's column lookup
		colofs = block + 8;
		texturecolumnofs[texnum] = colofs;
		blocktex = block;
		for (i = 0; i < texture->width; i++)
			colofs[i] += 3;
			
		// Now that the texture has been built in column cache,
		//  it is purgable from zone memory.
		Z_ChangeTag(block, PU_CACHE);
	}
	else						// Removes a GOTO
	{
		//
		// multi-patch textures (or 'composite')
		//
		blocksize = (texture->width * 4) + (texture->width * texture->height);
		
		//CONL_PrintF ("R_GenTex MULTI  %.8s size: %d\n",texture->name,blocksize);
		texturememory += blocksize;
		
		block = Z_Malloc(blocksize, PU_STATIC, &texturecache[texnum]);
		
		// columns lookup table
		colofs = block;
		texturecolumnofs[texnum] = colofs;
		
		// GhostlyDeath -- Read above! =)
		texturecache[texnum] = block;
		
		// texture data before the lookup table
		blocktex = block + (texture->width * 4);
		
		// Composite the columns together.
		patch = texture->patches;
		
		for (i = 0, patch = texture->patches; i < texture->patchcount; i++, patch++)
		{
			realpatch = W_CacheLumpNum(patch->patch, PU_CACHE);
			x1 = patch->originx;
			x2 = x1 + LittleSwapInt16(realpatch->width);
			
			if (x1 < 0)
				x = 0;
			else
				x = x1;
				
			if (x2 > texture->width)
				x2 = texture->width;
				
			for (; x < x2; x++)
			{
				patchcol = (column_t*) ((uint8_t*)realpatch + LittleSwapInt32(realpatch->columnofs[x - x1]));
				
				// generate column ofset lookup
				colofs[x] = (x * texture->height) + (texture->width * 4);
				
				R_DrawColumnInCache(patchcol, block + colofs[x], patch->originy, texture->height);
			}
		}
		
		// Now that the texture has been built in column cache,
		//  it is purgable from zone memory.
		Z_ChangeTag(block, PU_CACHE);
	}
	
	// GhostlyDeath <September 29, 2011> -- Flip
#if 0
	for (x = 0; x < texture->width; x++)
	{
		for (j = 0; j < texture->height / 2; j++)
		{
			x2 = *((uint8_t*)(texturecache[texnum] + (colofs[x] + (j))));
			*((uint8_t*)(texturecache[texnum] + (colofs[x] + (j)))) = *((uint8_t*)(texturecache[texnum] + (colofs[x] + (texture->height - j))));
			*((uint8_t*)(texturecache[texnum] + (colofs[x] + (texture->height - j)))) = x2;
		}
	}
#endif
	
	return blocktex;
}

//  convert flat to hicolor as they are requested
//
//uint8_t**  flatcache;

uint8_t* R_GetFlat(int flatlumpnum)
{
	return W_CacheLumpNum(flatlumpnum, PU_CACHE);
	
	/*  // this code work but is useless
	   uint8_t*    data;
	   short*   wput;
	   int      i,j;
	
	   //FIXME: work with run time pwads, flats may be added
	   // lumpnum to flatnum in flatcache
	   if ((data = flatcache[flatlumpnum-firstflat])!=0)
	   return data;
	
	   data = W_CacheLumpNum (flatlumpnum, PU_CACHE);
	   i=W_LumpLength(flatlumpnum);
	
	   Z_Malloc (i,PU_STATIC,&flatcache[flatlumpnum-firstflat]);
	   memcpy (flatcache[flatlumpnum-firstflat], data, i);
	
	   return flatcache[flatlumpnum-firstflat];
	 */
	
	/*  // this code don't work because it don't put a proper user in the z_block
	   if ((data = flatcache[flatlumpnum-firstflat])!=0)
	   return data;
	
	   data = (uint8_t *) W_CacheLumpNum(flatlumpnum,PU_LEVEL);
	   flatcache[flatlumpnum-firstflat] = data;
	   return data;
	
	   flatlumpnum -= firstflat;
	
	   if (scr_bpp==1)
	   {
	   flatcache[flatlumpnum] = data;
	   return data;
	   }
	
	   // allocate and convert to high color
	
	   wput = (short*) Z_Malloc (64*64*2,PU_STATIC,&flatcache[flatlumpnum]);
	   //flatcache[flatlumpnum] =(uint8_t*) wput;
	
	   for (i=0; i<64; i++)
	   for (j=0; j<64; j++)
	   wput[i*64+j] = ((color8to16[*data++]&0x7bde) + ((i<<9|j<<4)&0x7bde))>>1;
	
	   //Z_ChangeTag (data, PU_CACHE);
	
	   return (uint8_t*) wput;
	 */
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
			if (texturecache[i])
				Z_Free(texturecache[i]);
		}
}

/* RS_TexturePDRemove() -- Remove loaded texture data */
static void RS_TexturePDRemove(const struct WL_WADFile_s* a_WAD)
{
}

/* RS_TexturePDCreate() -- Create texture data for WADs */
static bool_t RS_TexturePDCreate(const struct WL_WADFile_s* const a_WAD, const uint32_t a_Key, void** const a_DataPtr, size_t* const a_SizePtr, WL_RemoveFunc_t* const a_RemoveFuncPtr)
{
	return false;
}

/* RS_TextureOrderChange() -- Order changed (rebuild composite) */
static bool_t RS_TextureOrderChange(const bool_t a_Pushed, const struct WL_WADFile_s* const a_WAD)
{
	return false;
}

/* R_LoadTextures() -- Loads texture data information */
// GhostlyDeath <December 14, 2011> -- Stubby
void R_LoadTextures(void)
{
	/* Register order callbacks and generators */
	// Generators first
	if (!WL_RegisterPDC(WLTEXTUREKEY, 100, RS_TexturePDCreate, RS_TextureOrderChange))
		I_Error("R_LoadTextures: Failed to register PDC.\n");
	
	// Order callback
	if (!WL_RegisterOCCB(RS_TextureOrderChange, 75))
		I_Error("R_LoadTextures: Failed to register OCCB.\n");

#if 0
	maptexture_t* mtexture;
	texture_t* texture;
	mappatch_t* mpatch;
	texpatch_t* patch;
	char* pnames;
	
	int i;
	int j;
	WadIndex_t k;
	int l;
	
	uint32_t* maptex;
	uint32_t* maptex2;
	uint32_t* maptex1;
	
	char name[9];
	char* name_p;
	
	WadIndex_t* patchlookup;
	
	uint32_t nummappatches;
	uint32_t offset;
	uint32_t maxoff;
	uint32_t maxoff2;
	uint32_t numtextures1;
	uint32_t numtextures2;
	
	uint32_t* directory;
	
	char* PS;
	char* PE;
	WadIndex_t PStart, PEnd;
	WadIndex_t v;
	WadFile_t* CurWad = NULL;
	WadIndex_t Potential, Found;
	int PotentialW;
	
	// free previous memory before numtextures change
	
	if (numtextures > 0)
		for (i = 0; i < numtextures; i++)
		{
			if (textures[i])
				Z_Free(textures[i]);
			if (texturecache[i])
				Z_Free(texturecache[i]);
		}
	// Load the patch names from pnames.lmp.
	name[8] = 0;
	pnames = W_CacheLumpName("PNAMES", PU_STATIC);
	nummappatches = LittleSwapInt32(*((uint32_t*)pnames));
	name_p = pnames + 4;
	patchlookup = Z_Malloc(nummappatches * sizeof(*patchlookup), PU_STATIC, NULL);
	
	for (i = 0; i < nummappatches; i++)
	{
		strncpy(name, name_p + i * 8, 8);
		
		// GhostlyDeath <December 20, 2008> -- USE what's between the P_ etc.!
		// instead of: patchlookup[i] = W_CheckNumForName(name);
		patchlookup[i] = INVALIDLUMP;
		Potential = Found = INVALIDLUMP;
		PotentialW = -1;
		
		// P1_, P2_, P3_, PP_
		
		for (j = W_NumWadFiles() - 1; j >= 0; j--)
		{
			CurWad = W_GetWadForNum(j);
			
			if (!CurWad)
				continue;
				
			/* Search the 4 containers */
			for (k = 0; k < 4; k++)
			{
				/* What are we searching between? */
				switch (k)
				{
					case 0:
						PS = "PP_START";
						PE = "PP_END";
						break;
					case 1:
						PS = "P3_START";
						PE = "P3_END";
						break;
					case 2:
						PS = "P2_START";
						PE = "P2_END";
						break;
					default:
						PS = "P1_START";
						PE = "P1_END";
						break;
				}
				
				PStart = INVALIDLUMP;
				PEnd = INVALIDLUMP;
				
				/* Get container locations */
				for (l = 0; l < CurWad->NumLumps; l++)
					if (strncasecmp(CurWad->Index[l].Name, PS, 8) == 0)
					{
						PStart = l;
						break;
					}
					
				for (; l < CurWad->NumLumps; l++)
					if (strncasecmp(CurWad->Index[l].Name, PE, 8) == 0)
					{
						PEnd = l;
						break;
					}
					
				/* Now search every entry... */
				if (PStart != INVALIDLUMP && PEnd != INVALIDLUMP)
					for (v = (PStart + 1); v <= (PEnd - 1); v++)
						if (strncasecmp(CurWad->Index[v].Name, name, 8) == 0)
						{
							if (PotentialW > -1 && PotentialW >= W_GetNumForWad(CurWad))
								Found = Potential;
							else
								Found = W_LumpsSoFar(CurWad) + v;
							break;
						}
						
				if (Found != INVALIDLUMP)
					break;
			}
			
			if (Found != INVALIDLUMP)
				break;
				
			// GhostlyDeath <August 27, 2011> -- Potential lump
			// This fixes patches in DWANGO5.WAD that are set
			if (Potential == INVALIDLUMP)
			{
				Potential = W_CheckNumForNamePwadPtr(name, CurWad, 0);
				
				if (Potential != INVALIDLUMP)
					PotentialW = W_GetNumForWad(CurWad);
				break;
			}
		}
		
		// IF Nothing was found... resort to Legacy mode...
		if (Found == INVALIDLUMP)
			// GhostlyDeath <August 27, 2011> -- Is there potential?
			if (Potential != INVALIDLUMP)
				patchlookup[i] = Potential;
			else
				patchlookup[i] = W_CheckNumForName(name);
		else
			patchlookup[i] = Found;
	}
	
	// Free temporary (ouch here!)
	Z_Free(pnames);
	
	// Load the map texture definitions from textures.lmp.
	// The data is contained in one or two lumps,
	//  TEXTURE1 for shareware, plus TEXTURE2 for commercial.
	maptex = maptex1 = W_CacheLumpName("TEXTURE1", PU_STATIC);
	numtextures1 = *maptex;
	maxoff = W_LumpLength(W_GetNumForName("TEXTURE1"));
	directory = maptex + 1;
	
	if (W_CheckNumForName("TEXTURE2") != -1)
	{
		maptex2 = W_CacheLumpName("TEXTURE2", PU_STATIC);
		numtextures2 = *maptex2;
		maxoff2 = W_LumpLength(W_GetNumForName("TEXTURE2"));
	}
	else
	{
		maptex2 = NULL;
		numtextures2 = 0;
		maxoff2 = 0;
	}

	// GhostlyDeath <November 4, 2011> -- Really need new texture code here of sorts
	numtextures1 = LittleSwapUInt32(numtextures1);
	numtextures2 = LittleSwapUInt32(numtextures2);

	numtextures = numtextures1 + numtextures2;
	
	// GhostlyDeath -- Due to the new code, we clear all the buffers!
	if (textures)
		Z_Free(textures);
	if (texturecolumnofs)
		Z_Free(texturecolumnofs);
	if (texturecache);
	Z_Free(texturecache);
	if (texturewidthmask);
	Z_Free(texturewidthmask);
	if (textureheight)
		Z_Free(textureheight);
		
	/*// GhostlyDeath -- OK after reading... textures is a pointer list
	   textures = Z_Malloc(((numtextures * sizeof(texture_t*)) * 5) + sizeof(texture_t*), PU_STATIC, 0);
	   //textures         = Z_Malloc((numtextures*sizeof(size_t)*5)+1, PU_STATIC, 0);
	
	   texturecolumnofs = (void*)(((size_t*)textures) + (((size_t)numtextures) * 1));
	   texturecache         = (void*)(((size_t*)textures) + (((size_t)numtextures) * 2));
	   texturewidthmask = (void*)(((size_t*)textures) + (((size_t)numtextures) * 3));
	   textureheight        = (void*)(((size_t*)textures) + (((size_t)numtextures) * 4)); */
	
	// GhostlyDeath -- Fuck all of it, WHY DONT WE JUST MAKE THEM INDIV BUFFERS!?
	/*
	   FOR REFERENCE:
	
	   texture_t** textures=NULL;
	   uint32_t** texturecolumnofs;   // column offset lookup table for each texture
	   uint8_t** texturecache;       // graphics data for each generated full-size texture
	   int* texturewidthmask;   // texture width is a power of 2, so it can easily repeat along sidedefs using a simple mask
	   fixed_t* textureheight;      // needed for texture pegging
	 */
	textures = Z_Malloc(numtextures * sizeof(texture_t*), PU_STATIC, 0);
	memset(textures, 0, numtextures * sizeof(texture_t*));
	texturecolumnofs = Z_Malloc(numtextures * sizeof(uint32_t*), PU_STATIC, 0);
	memset(textures, 0, numtextures * sizeof(uint32_t*));
	texturecache = Z_Malloc(numtextures * sizeof(uint8_t*), PU_STATIC, 0);
	memset(textures, 0, numtextures * sizeof(uint8_t*));
	texturewidthmask = Z_Malloc(numtextures * sizeof(int), PU_STATIC, 0);
	memset(textures, 0, numtextures * sizeof(int));
	textureheight = Z_Malloc(numtextures * sizeof(fixed_t), PU_STATIC, 0);
	memset(textures, 0, numtextures * sizeof(fixed_t));
	
	for (i = 0; i < numtextures; i++, directory++)
	{
		//only during game startup
		//if (!(i&63))
		//    CONL_PrintF (".");
		
		if (i == numtextures1)
		{
			// Start looking in second texture file.
			maptex = maptex2;
			maxoff = maxoff2;
			directory = maptex + 1;
		}
		// offset to the current texture in TEXTURESn lump
		offset = *directory;
		offset = LittleSwapUInt32(offset);	// FIXME
		
		if (offset > maxoff)
			I_Error("R_LoadTextures: bad texture directory");
			
		// maptexture describes texture name, size, and
		// used patches in z order from bottom to top
		mtexture = (maptexture_t*) ((uint8_t*)maptex + offset);
		
		if (!mtexture && devparm)
			CONL_PrintF("R_LoadTextures: Warning mtexture is NULL!");
			
		textures[i] = Z_Malloc(sizeof(texture_t) + sizeof(texpatch_t) * (mtexture->patchcount), PU_STATIC, 0);
		/*if (devparm)
		   CONL_PrintF("Z_Malloc(%i + %i * %i) = %i\n", sizeof(texture_t), sizeof(texpatch_t), (mtexture->patchcount),
		   sizeof(texture_t) + sizeof(texpatch_t) * (mtexture->patchcount)); */
		
		texture = textures[i];
		
		texture->width = LittleSwapInt16(mtexture->width);
		texture->height = LittleSwapInt16(mtexture->height);
		texture->patchcount = LittleSwapInt16(mtexture->patchcount);
		
		// Sparc requires memmove, becuz gcc doesn't know mtexture is not aligned.
		// gcc will replace memcpy with two 4-uint8_t read/writes, which will bus error.
		memmove(texture->name, mtexture->name, sizeof(texture->name));
		mpatch = &mtexture->patches[0];
		patch = &texture->patches[0];
		
		for (j = 0; j < texture->patchcount; j++, mpatch++, patch++)
		{
			patch->originx = LittleSwapInt16(mpatch->originx);
			patch->originy = LittleSwapInt16(mpatch->originy);
			patch->patch = patchlookup[LittleSwapInt16(mpatch->patch)];
			if (patch->patch == -1)
			{
				I_Error("R_InitTextures: Missing patch in texture %s (tried %i)", texture->name, mpatch->patch);
			}
		}
		
		j = 1;
		while (j * 2 <= texture->width)
			j <<= 1;
			
		texturewidthmask[i] = j - 1;
		textureheight[i] = texture->height << FRACBITS;
	}
	
	Z_Free(maptex1);
	if (maptex2)
		Z_Free(maptex2);
		
	//added:01-04-98: this takes 90% of texture loading time..
	// Precalculate whatever possible.
	for (i = 0; i < numtextures; i++)
		texturecache[i] = NULL;
		
	// Create translation table for global animation.
	if (texturetranslation)
		Z_Free(texturetranslation);
		
	texturetranslation = Z_Malloc((numtextures + 1) * sizeof(void*), PU_STATIC, 0);
	
	for (i = 0; i < numtextures; i++)
		texturetranslation[i] = i;
#endif
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
#if 0
	WadIndex_t startnum;
	WadIndex_t endnum;
	int onef;
	WadFile_t* wad;
	uint32_t i;
	
	numflatlists = 0;
	flats = NULL;
	
	for (i = 0; i < W_NumWadFiles(); i++)
	{
		wad = W_GetWadForNum(i);
		
		startnum = endnum = INVALIDLUMP;
		
		onef = 0;
		startnum = W_CheckNumForNamePwadPtr("F_START", wad, 0);
		
		if (startnum == INVALIDLUMP)
			startnum = W_CheckNumForNamePwadPtr("FF_START", wad, 0);
		else
			onef = 1;
			
		// GhostlyDeath <June 21, 2009> -- Deutex and such does not use FF_END, it uses F_END! wtf!
		if (startnum != INVALIDLUMP)
		{
			// F_START never ends in FF_END!
			if (!onef)
				endnum = W_CheckNumForNamePwadPtr("FF_END", wad, 0);
				
			if (onef || endnum == INVALIDLUMP)
				endnum = W_CheckNumForNamePwadPtr("F_END", wad, 0);
		}
		
		if (startnum != INVALIDLUMP && endnum != INVALIDLUMP && endnum > startnum)
		{
			CONL_PrintF("R_InitFlats: Registered %i flats in %s.\n", endnum - startnum - 1, wad->FileName);
			flats = (lumplist_t*) realloc(flats, sizeof(lumplist_t) * (numflatlists + 1));
			flats[numflatlists].WadFile = wad;
			flats[numflatlists].firstlump = startnum - W_LumpsSoFar(wad);
			flats[numflatlists].numlumps = (endnum - startnum);
			numflatlists++;
		}
	}
	
	if (!numflatlists)
		I_Error("R_InitFlats: No flats found!\n");
#endif
}

/* R_GetFlatNumForName() -- Find flat by it's name */
int R_GetFlatNumForName(char* name)
{
	// GhostlyDeath <Sunday, June 21, 2009> -- B Pierra said that R_CheckNumForNameList()
	//     does not work with gothic2.wad...
	WadIndex_t Lump = INVALIDLUMP;
	
	Lump = R_CheckNumForNameList(name, flats, numflatlists);
	
	if (Lump == INVALIDLUMP)
		Lump = W_CheckNumForName(name);
		
	// Instead of exploding, why don't we load an "invalid" flat like a checker pattern?
	if (Lump == INVALIDLUMP)
		Lump = R_CheckNumForNameList("-NOFLAT-", flats, numflatlists);
	//I_Error("R_GetFlatNumForName: Could not find flat \"%.8s\".\n");
	
	return Lump;
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
	CONL_PrintF("\nInitTextures...");
	R_LoadTextures();
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
// R_CheckTextureNumForName
// Check whether texture is available.
// Filter out NoTexture indicator.
//
int R_CheckTextureNumForName(char* name)
{
	int i;
	
	// "NoTexture" marker.
	if (name[0] == '-')
		return 0;
		
	for (i = 0; i < numtextures; i++)
	{
		if (!textures[i]->name)
			continue;
			
		if (!strncasecmp(textures[i]->name, name, 8))
			return i;
	}
	
	return -1;
}

//
// R_TextureNumForName
// Calls R_CheckTextureNumForName,
//  aborts with error message.
//
int R_TextureNumForName(char* name)
{
	int i;
	
	i = R_CheckTextureNumForName(name);
	
	if (i == -1)
	{
		//I_Error ("R_TextureNumForName: %.8s not found", name);
		CONL_PrintF("WARNING: R_TextureNumForName: %.8s not found\n", name);
		return 1;
	}
	return i;
}

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
		if (texturecache[i] == NULL)
			R_GenerateTexture(i);
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
