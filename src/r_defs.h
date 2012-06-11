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
// DESCRIPTION: Refresh/rendering module, shared data struct definitions.

#ifndef __R_DEFS__
#define __R_DEFS__

// Some more or less basic data types
// we depend on.
#include "m_fixed.h"

// We rely on the thinker data struct
// to handle sound origins in sectors.
#include "d_think.h"
// SECTORS do store MObjs anyway.
#include "p_mobj.h"

#include "screen.h"

// Silhouette, needed for clipping Segs (mainly)
// and sprites representing things.
#define SIL_NONE                0
#define SIL_BOTTOM              1
#define SIL_TOP                 2
#define SIL_BOTH                3

//faB: was upped to 512, but still people come with levels that break the
//     limits, so had to do an ugly re-alloc to get rid of the overflow.
//#define MAXDRAWSEGS             256        // see r_segs.c for more

// SoM: Moved this here...
// This could be wider for >8 bit display.
// Indeed, true color support is posibble
//  precalculating 24bpp lightmap/colormap LUT.
//  from darkening PLAYPAL to all black.
// Could even us emore than 32 levels.
typedef uint8_t lighttable_t;

// SoM: ExtraColormap type. Use for extra_colormaps from now on.
typedef struct
{
	unsigned short maskcolor;
	unsigned short fadecolor;
	double maskamt;
	unsigned short fadestart, fadeend;
	int fog;
	
	//Hurdler: rgba is used in hw mode for coloured sector lighting
	int rgba;					// similar to maskcolor in sw mode
	
	lighttable_t* colormap;
} extracolormap_t;

//
// INTERNAL MAP TYPES
//  used by play and refresh
//

//
// Your plain vanilla vertex.
// Note: transformed values not buffered locally,
//  like some DOOM-alikes ("wt", "WebView") did.
//
typedef struct
{
	fixed_t x;
	fixed_t y;
	
} vertex_t;

// Forward of LineDefs, for Sectors.
struct line_s;

// Each sector has a degenmobj_t in its center
//  for sound origin purposes.
// I suppose this does not handle sound from
//  moving objects (doppler), because
//  position is prolly just buffered, not
//  updated.
typedef struct
{
	thinker_t thinker;			// not used for anything
	fixed_t x;
	fixed_t y;
	fixed_t z;
	
} degenmobj_t;

//SoM: 3/23/2000: Store fake planes in a resizalbe array insted of just by
//heightsec. Allows for multiple fake planes.
typedef enum
{
	FF_EXISTS = 0x1,			//MAKE SURE IT'S VALID
	FF_SOLID = 0x2,				//Does it clip things?
	FF_RENDERSIDES = 0x4,		//Render the sides?
	FF_RENDERPLANES = 0x8,		//Render the floor/ceiling?
	FF_RENDERALL = 0xC,			//Render everything?
	FF_SWIMMABLE = 0x10,		//Can we swim?
	FF_NOSHADE = 0x20,			//Does it mess with the lighting?
	FF_CUTSOLIDS = 0x40,		//Does it cut out hidden solid pixles?
	FF_CUTEXTRA = 0x80,			//Does it cut out hidden translucent pixles?
	FF_CUTLEVEL = 0xC0,			//Does it cut out all hidden pixles?
	FF_CUTSPRITES = 0x100,		//Final Step in 3D water
	FF_BOTHPLANES = 0x200,		//Render both planes all the time?
	FF_EXTRA = 0x400,			//Does it get cut by FF_CUTEXTRAS?
	FF_TRANSLUCENT = 0x800,		//See through!
	FF_FOG = 0x1000,			//Fog "brush"?
	FF_INVERTPLANES = 0x2000,	//Reverse the plane visibility rules?
	FF_ALLSIDES = 0x4000,		//Render inside and outside sides?
	FF_INVERTSIDES = 0x8000,	//Only render inside sides?
	FF_DOUBLESHADOW = 0x10000,	//Make two lightlist entries to reset light?
} ffloortype_e;

typedef struct ffloor_s
{
	fixed_t* topheight;
	short* toppic;
	short* toplightlevel;
	fixed_t* topxoffs;
	fixed_t* topyoffs;
	
	fixed_t* bottomheight;
	short* bottompic;
	//short            *bottomlightlevel;
	fixed_t* bottomxoffs;
	fixed_t* bottomyoffs;
	
	fixed_t delta;
	
	int secnum;
	ffloortype_e flags;
	struct line_s* master;
	
	struct sector_s* target;
	
	struct ffloor_s* next;
	struct ffloor_s* prev;
	
	int lastlight;
	int alpha;
	
	struct mobj_s* OwnerMobj;
} ffloor_t;

// GhostlyDeath <May 10, 2012> -- Fake Floor List (For Savegames)
extern ffloor_t** g_PFakeFloors;				// Fake Floors
extern size_t g_NumPFakeFloors;					// Number of them

// SoM: This struct holds information for shadows casted by 3D floors.
// This information is contained inside the sector_t and is used as the base
// information for casted shadows.
typedef struct lightlist_s
{
	fixed_t height;
	short* lightlevel;
	extracolormap_t* extra_colormap;
	int flags;
	ffloor_t* caster;
} lightlist_t;

// SoM: This struct is used for rendering walls with shadows casted on them...
typedef struct r_lightlist_s
{
	fixed_t height;
	fixed_t heightstep;
	fixed_t botheight;
	fixed_t botheightstep;
	short lightlevel;
	extracolormap_t* extra_colormap;
	lighttable_t* rcolormap;
	int flags;
	int lightnum;
} r_lightlist_t;

typedef enum
{
	FLOOR_SOLID,
	FLOOR_WATER,
	FLOOR_LAVA,
	FLOOR_SLUDGE,
	FLOOR_ICE,
} floortype_t;

// ----- for special tricks with HW renderer -----

//
// For creating a chain with the lines around a sector
//
typedef struct linechain_s
{
	struct line_s* line;
	struct linechain_s* next;
} linechain_t;

// ----- end special tricks -----

//
// The SECTORS record, at runtime.
// Stores things/mobjs.
//
typedef struct sector_s
{
	fixed_t floorheight;
	fixed_t ceilingheight;
	short floorpic;
	short ceilingpic;
	short lightlevel;
	uint32_t special;
	uint32_t oldspecial;			//SoM: 3/6/2000: Remember if a sector was secret (for automap)
	short tag;
	int nexttag, firsttag;		//SoM: 3/6/2000: by killough: improves searches for tags.
	
	// 0 = untraversed, 1,2 = sndlines -1
	short soundtraversed;
	short floortype;			// see floortype_t beffor
	
	// thing that made a sound (or null)
	mobj_t* soundtarget;
	
	// mapblock bounding box for height changes
	int blockbox[4];
	
	// origin for any sounds played by the sector
	S_NoiseThinker_t soundorg;
	
	// if == validcount, already checked
	int validcount;
	
	// list of mobjs in sector
	mobj_t* thinglist;
	
	//SoM: 3/6/2000: Start boom extra stuff
	// thinker_t for reversable actions
	void* floordata;			// make thinkers on
	void* ceilingdata;			// floors, ceilings, lighting,
	void* lightingdata;			// independent of one another
	
	// lockout machinery for stairbuilding
	int stairlock;				// -2 on first locked -1 after thinker done 0 normally
	int prevsec;				// -1 or number of sector for previous step
	int nextsec;				// -1 or number of next step sector
	
	// floor and ceiling texture offsets
	fixed_t floor_xoffs, floor_yoffs;
	fixed_t ceiling_xoffs, ceiling_yoffs;
	
	int heightsec;				// other sector, or -1 if no other sector
	int altheightsec;			// Use old boom model? 1 for no 0 for yes.
	
	int floorlightsec, ceilinglightsec;
	int teamstartsec;
	
	int bottommap, midmap, topmap;	// dynamic colormaps
	
	// list of mobjs that are at least partially in the sector
	// thinglist is a subset of touching_thinglist
	struct msecnode_s* touching_thinglist;	// phares 3/14/98
	//SoM: 3/6/2000: end stuff...
	
	int linecount;
	struct line_s** lines;		// [linecount] size
	
	//SoM: 2/23/2000: Improved fake floor hack
	ffloor_t* ffloors;
	int* attached;
	int numattached;
	
	bool_t LLSelf;								// True if lightlist is Z_Malloc()
	lightlist_t* lightlist;
	int numlights;
	
	bool_t moved;
	
	int validsort;				//if == validsort allready been sorted
	bool_t added;
	
	// SoM: 4/3/2000: per-sector colormaps!
	extracolormap_t* extra_colormap;
	
	// ----- for special tricks with HW renderer -----
	bool_t pseudoSector;
	bool_t virtualFloor;
	fixed_t virtualFloorheight;
	bool_t virtualCeiling;
	fixed_t virtualCeilingheight;
	linechain_t* sectorLines;
	struct sector_s** stackList;
	double lineoutLength;
	// ----- end special tricks -----
	
	// ReMooD Additions
	char* FloorTexture;							// Name of floor texture
	char* CeilingTexture;						// Name of ceiling texture
	fixed_t BBox[4];							// Sector bounding box
	size_t SoundSecRef;							// Reference to sound sector
	int32_t AltSkyTexture;						// Alternate Sky Texture
	bool_t AltSkyFlipped;						// Flipped Alternate Sky
	
	struct sector_s** Adj;						// Adjacent sectors
	size_t NumAdj;								// Number of adjacent sectors
} sector_t;

//
// The SideDef.
//

typedef struct
{
	// add this to the calculated texture column
	fixed_t textureoffset;
	
	// add this to the calculated texture top
	fixed_t rowoffset;
	
	// Texture indices.
	// We do not maintain names here.
	short toptexture;
	short bottomtexture;
	short midtexture;
	
	// Sector the SideDef is facing.
	sector_t* sector;
	
	//SoM: 3/6/2000: This is the special of the linedef this side belongs to.
	uint32_t special;
	
	// GhostlyDeath <September 29, 2011> -- Cool Texture Stuff
	fixed_t ScaleX, ScaleY;
	bool_t VFlip;				// vertically flip texture
	
	// ReMooD Additions
	char* WallTextures[3];						// Textures used on walls
	size_t SectorNum;							// Sector number linked to
} side_t;

//
// Move clipping aid for LineDefs.
//
typedef enum
{
	ST_HORIZONTAL,
	ST_VERTICAL,
	ST_POSITIVE,
	ST_NEGATIVE
} slopetype_t;

typedef struct line_s
{
	// Vertices, from v1 to v2.
	vertex_t* v1;
	vertex_t* v2;
	
	// Precalculated v2 - v1 for side checking.
	fixed_t dx;
	fixed_t dy;
	
	// Animation related.
	short flags;
	uint32_t special;
	short tag;
	
	// Visual appearance: SideDefs.
	//  sidenum[1] will be -1 if one sided
	short sidenum[2];
	
	// Neat. Another bounding box, for the extent
	//  of the LineDef.
	fixed_t bbox[4];
	
	// To aid move clipping.
	slopetype_t slopetype;
	
	// Front and back sector.
	// Note: redundant? Can be retrieved from SideDefs.
	sector_t* frontsector;
	sector_t* backsector;
	
	// if == validcount, already checked
	int validcount;
	
	// thinker_t for reversable actions
	void* specialdata;
	
	// wallsplat_t list
	void* splats;
	
	//SoM: 3/6/2000
	int tranlump;				// translucency filter, -1 == none
	// (Will have to fix to use with Legacy's Translucency?)
	int firsttag, nexttag;		// improves searches for tags.
	
	int ecolormap;				// SoM: Used for 282 linedefs
	
	// ReMooD Additions
	size_t VertexNum[2];						// IDs for vertexes
	
	// Hexen
	uint8_t HexenSpecial;						// Hexen special ID
	uint8_t ACSArgs[5];							// Arguments for ACS lines
} line_t;

//
// A SubSector.
// References a Sector.
// Basically, this is a list of LineSegs,
//  indicating the visible walls that define
//  (all or some) sides of a convex BSP leaf.
//
typedef struct subsector_s
{
	sector_t* sector;
	short numlines;
	short firstline;
	// floorsplat_t list
	void* splats;
	//Hurdler: added for optimized mlook in hw mode
	int validcount;
	
	// GhostlyDeath <April 23, 2012> -- Bots
	bool_t NodesInit;							// Subsector mapped
	void** BotNodes;							// Node in subsector
	size_t NumBotNodes;							// Number of nodes in this subsector
	
	void** GhostNodes;
	size_t NumGhostNodes;
} subsector_t;

// SoM: 3/6/200
//
// Sector list node showing all sectors an object appears in.
//
// There are two threads that flow through these nodes. The first thread
// starts at touching_thinglist in a sector_t and flows through the m_snext
// links to find all mobjs that are entirely or partially in the sector.
// The second thread starts at touching_sectorlist in an mobj_t and flows
// through the m_tnext links to find all sectors a thing touches. This is
// useful when applying friction or push effects to sectors. These effects
// can be done as thinkers that act upon all objects touching their sectors.
// As an mobj moves through the world, these nodes are created and
// destroyed, with the links changed appropriately.
//
// For the links, NULL means top or end of list.

typedef struct msecnode_s
{
	sector_t* m_sector;			// a sector containing this object
	struct mobj_s* m_thing;		// this object
	struct msecnode_s* m_tprev;	// prev msecnode_t for this thing
	struct msecnode_s* m_tnext;	// next msecnode_t for this thing
	struct msecnode_s* m_sprev;	// prev msecnode_t for this sector
	struct msecnode_s* m_snext;	// next msecnode_t for this sector
	bool_t visited;				// killough 4/4/98, 4/7/98: used in search algorithms
} msecnode_t;

extern msecnode_t** g_MSecNodes;				// Active sector nodes
extern size_t g_NumMSecNodes;					// Number of sector nodes

//Hurdler: 04/12/2000: for now, only used in hardware mode
//                     maybe later for software as well?
//                     that's why it's moved here
typedef struct light_s
{
	uint16_t type;				// light,... (cfr #define in hwr_light.c)
	
	float light_xoffset;
	float light_yoffset;		// y offset to adjust corona's height
	
	uint32_t corona_color;		// color of the light for static lighting
	float corona_radius;		// radius of the coronas
	
	uint32_t dynamic_color;		// color of the light for dynamic lighting
	float dynamic_radius;		// radius of the light ball
	float dynamic_sqrradius;	// radius^2 of the light ball
	
} light_t;

typedef struct lightmap_s
{
	float s[2], t[2];
	light_t* light;
	struct lightmap_s* next;
} lightmap_t;

//
// The LineSeg.
//
typedef struct
{
	vertex_t* v1;
	vertex_t* v2;
	
	int side;
	
	fixed_t offset;
	
	angle_t angle;
	
	side_t* sidedef;
	line_t* linedef;
	
	// Sector references.
	// Could be retrieved from linedef, too.
	// backsector is NULL for one sided lines
	sector_t* frontsector;
	sector_t* backsector;
	
	// lenght of the seg : used by the hardware renderer
	float length;
	
	//Hurdler: 04/12/2000: added for static lightmap
	lightmap_t* lightmaps;
	
	// SoM: Why slow things down by calculating lightlists for every
	// thick side.
	int numlights;
	r_lightlist_t* rlights;
	
	// ReMooD Additions
	uint32_t VertexID[2];
	uint32_t LineID;
} seg_t;

//
// BSP node.
//
typedef struct
{
	// Partition line.
	fixed_t x;
	fixed_t y;
	fixed_t dx;
	fixed_t dy;
	
	// Bounding box for each child.
	fixed_t bbox[2][4];
	
	// If NF_SUBSECTOR its a subsector.
	uint16_t children[2];
	
} node_t;

// posts are runs of non masked source pixels
typedef struct
{
	uint8_t topdelta;			// -1 is the last post in a column
	// BP: humf, -1 with uint8_t ! (unsigned char) test WARNING
	uint8_t length;				// length data bytes follows
} post_t;

// column_t is a list of 0 or more post_t, (uint8_t)-1 terminated
typedef post_t column_t;

//
// OTHER TYPES
//

#ifndef MAXFFLOORS
#define MAXFFLOORS    40
#endif

//
// ?
//
typedef struct drawseg_s
{
	seg_t* curline;
	int x1;
	int x2;
	
	fixed_t scale1;
	fixed_t scale2;
	fixed_t scalestep;
	
	// 0=none, 1=bottom, 2=top, 3=both
	int silhouette;
	
	// do not clip sprites above this
	fixed_t bsilheight;
	
	// do not clip sprites below this
	fixed_t tsilheight;
	
	// Pointers to lists for sprite clipping,
	//  all three adjusted so [x1] is first value.
	short* sprtopclip;
	short* sprbottomclip;
	short* maskedtexturecol;
	
	struct visplane_s* ffloorplanes[MAXFFLOORS];
	int numffloorplanes;
	struct ffloor_s* thicksides[MAXFFLOORS];
	short* thicksidecol;
	int numthicksides;
	fixed_t* frontscale;
} drawseg_t;

// Patches.
// A patch holds one or more columns.
// Patches are used for sprites and all masked pictures,
// and we compose textures from the TEXTURE1/2 lists
// of patches.
//
struct patch_s
{
	short width;				// bounding box size
	short height;
	short leftoffset;			// pixels to the left of origin
	short topoffset;			// pixels below the origin
	uint32_t columnofs[8];		// only [width] used
	// the [0] is &columnofs[width]
};
typedef struct patch_s patch_t;

#if 0
typedef enum
{
	PALETTE = 0,				// 1 uint8_t is the index in the doom palette (as usual)
	INTENSITY = 1,				// 1 uint8_t intensity
	INTENSITY_ALPHA = 2,		// 2 uint8_t : alpha then intensity
	RGB24 = 3,					// 24 bit rgb
	RGBA32 = 4,					// 32 bit rgba
} pic_mode_t;
#endif
// a pic is an unmasked block of pixels, stored in horizontal way
//
typedef struct
{
	int16_t width;
	uint8_t zero;				// set to 0 allow autodetection of pic_t
	// mode instead of patch or raw
	uint8_t mode;				// see pic_mode_t above
	int16_t height;
	int16_t reserved1;			// set to 0
	uint8_t data[0];
} pic_t;

typedef enum
{
	SC_NONE = 0,
	SC_TOP = 1,
	SC_BOTTOM = 2
} spritecut_e;

// A vissprite_t is a thing
//  that will be drawn during a refresh.
// I.e. a sprite object that is partly visible.
typedef struct vissprite_s
{
	// Doubly linked list.
	struct vissprite_s* prev;
	struct vissprite_s* next;
	
	int x1;
	int x2;
	
	// for line side calculation
	fixed_t gx;
	fixed_t gy;
	
	// global bottom / top for silhouette clipping
	fixed_t gz;
	fixed_t gzt;
	
	// Physical bottom / top for sorting with 3D floors.
	fixed_t pz;
	fixed_t pzt;
	
	// horizontal position of x1
	fixed_t startfrac;
	
	fixed_t scale;
	
	// negative if flipped
	fixed_t xiscale;
	
	fixed_t texturemid;
	int patch;
	
	// for color translation and shadow draw,
	//  maxbright frames as well
	lighttable_t* colormap;
	
	//Fab:29-04-98: for MF_SHADOW sprites, which translucency table to use
	uint8_t* transmap;
	
	uint32_t mobjflags;
	uint32_t mobjflags2;
	uint32_t RXFlags[NUMINFORXFIELDS];
	int MoSkinColor;
	
	// SoM: 3/6/2000: height sector for underwater/fake ceiling support
	int heightsec;
	
	//SoM: 4/3/2000: Global colormaps!
	extracolormap_t* extra_colormap;
	fixed_t xscale;
	
	//SoM: Precalculated top and bottom screen coords for the sprite.
	fixed_t thingheight;		//The actual height of the thing (for 3D floors)
	sector_t* sector;			//The sector containing the thing.
	fixed_t sz;
	fixed_t szt;
	
	int cut;					//0 for none, bit 1 for top, bit 2 for bottom
	
	/* Render Optimization */
	fixed_t Distance;			// Distance of sprite
	int Priority;				// Priority of object to be seen
	int BasePriority;			// Base Priority of object
	
	// GhostlyDeath <February 24, 2012> -- Image to draw
	void* Image;								// Image to draw for sprite
} vissprite_t;

/* R_SpriteInfoEx_t -- Extended sprite information */
typedef struct R_SpriteInfoEx_s
{
	char Name[5];								// Sprite name
	uint32_t Code;								// Sprite code name
	int8_t Frame[2];							// Frame of sprite (2 possible)
	int8_t Rotation[2];							// Rotation (0 = all, 1.. = rest)
	bool_t Double;								// Double frames
	bool_t Init;								// Data initialized?
	
	fixed_t Width;								// Width of sprite
	fixed_t Offset;								// Offset of sprite (x axis)
	fixed_t TopOffset;							// Offset of sprite (y axis)
	fixed_t Height;								// Height of sprite
	
	struct V_Image_s* Image;					// Cached image info
	struct WL_WADEntry_s* Entry;				// Entry for sprite image
} R_SpriteInfoEx_t;

//
// Sprites are patches with a special naming convention
//  so they can be recognized by R_InitSprites.
// The base name is NNNNFx or NNNNFxFx, with
//  x indicating the rotation, x = 0, 1-7.
// The sprite and frame specified by a thing_t
//  is range checked at run time.
// A sprite is a patch_t that is assumed to represent
//  a three dimensional object and may have multiple
//  rotations pre drawn.
// Horizontal flipping is used to save space,
//  thus NNNNF2F5 defines a mirrored patch.
// Some sprites will only have one picture used
// for all views: NNNNF0
//
typedef struct
{
	// If false use 0 for any position.
	// Note: as eight entries are available,
	//  we might as well insert the same name eight times.
	bool_t rotate;
	
	// Lump to use for view angles 0-7.
	int lumppat[8];				// lump number 16:16 wad:lump
	short lumpid[8];			// id in the spriteoffset,spritewidth.. tables
	
	// Flip bit (1 = flip) to use for view angles 0-7.
	uint8_t flip[8];
	
	// GhostlyDeath <February 21, 2012> -- Extended dynamic sprites
	R_SpriteInfoEx_t* ExAngles[16];				// Extended frame pointers
	bool_t ExFlip[16];							// Flip sprites?
} spriteframe_t;

//
// A sprite definition:  a number of animation frames.
//
typedef struct
{
	int numframes;
	spriteframe_t* spriteframes;
	
	uint32_t Code;								// Sprite code name
} spritedef_t;

#define BORIS_FIX
#ifdef BORIS_FIX
extern short* last_ceilingclip;
extern short* last_floorclip;
#endif

typedef struct
{
	int first;
	int last;
	
} cliprange_t;

//SoM: 3/28/2000: Fix from boom.
#define MAXSEGS (vid.width/2+1)

extern cliprange_t* solidsegs;
extern drawseg_t* drawsegs;

extern fixed_t* cachedheight;
extern fixed_t* cacheddistance;
extern fixed_t* cachedxstep;
extern fixed_t* cachedystep;

#endif

