// -*- Mode: Java; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-
// ---------------------------------------------------------------------------
// ReMooD Doom Source Port <http://remood.org/>
//   Copyright (C) 2005-2016 GhostlyDeath <ghostlydeath@remood.org>
//     For more credits, see readme.mkd.
// ---------------------------------------------------------------------------
// ReMooD is under the GNU General Public License v3+, see license.mkd.
// ---------------------------------------------------------------------------

package org.remood.remood.core.random;

/**
 * This class provides the random number generator which Doom uses to generate
 * randomized data.
 *
 * @since 2016/05/07
 */
public class DoomRandom
{
	/** This is Doom's random number table. */
	private static final byte[] _RANDOM_TABLE =
		{
		   0,    8,  109,  -36,  -34,  -15, -107,  107, 
		  75,   -8,   -2, -116,   16,   66,   74,   21, 
		 -45,   47,   80,  -14, -102,   27,  -51, -128, 
		 -95,   89,   77,   36,   95,  110,   85,   48, 
		 -44, -116,  -45,   -7,   22,   79,  -56,   50, 
		  28,  -68,   52, -116,  -54,  120,   68, -111, 
		  62,   70,  -72,  -66,   91,  -59, -104,  -32, 
		-107,  104,   25,  -78,   -4,  -74,  -54,  -74, 
		-115,  -59,    4,   81,  -75,  -14, -111,   42, 
		  39,  -29, -100,  -58,  -31,  -63,  -37,   93, 
		 122,  -81,   -7,    0,  -81, -113,   70,  -17, 
		  46,  -10,  -93,   53,  -93,  109,  -88, -121, 
		   2,  -21,   25,   92,   20, -111, -118,   77, 
		  69,  -90,   78,  -80,  -83,  -44,  -90,  113, 
		  94,  -95,   41,   50,  -17,   49,  111,  -92, 
		  70,   60,    2,   37,  -85,   75, -120, -100, 
		  11,   56,   42, -110, -118,  -27,   73, -110, 
		  77,   61,   98,  -60, -121,  106,   63,  -59, 
		 -61,   86,   96,  -53,  113,  101,  -86,   -9, 
		 -75,  113,   80,   -6,  108,    7,   -1,  -19, 
		-127,  -30,   79,  107,  112,  -90,  103,  -15, 
		  24,  -33,  -17,  120,  -58,   58,   60,   82, 
		-128,    3,  -72,   66, -113,  -32, -111,  -32, 
		  81,  -50,  -93,   45,   63,   90,  -88,  114, 
		  59,   33,  -97,   95,   28, -117,  123,   98, 
		 125,  -60,   15,   70,  -62,   -3,   54,   14, 
		 109,  -30,   71,   17,  -95,   93,  -70,   87, 
		 -12, -118,   20,   52,  123,   -5,   26,   36, 
		  17,   46,   52,  -25,  -24,   76,   31,  -35, 
		  84,   37,  -40,  -91,  -44,  106,  -59,  -14, 
		  98,   43,   39,  -81,   -2, -111,  -66,   84, 
		 118,  -34,  -69, -120,  120,  -93,  -20,   -7
		};
	
	/** The current random index. */
	private volatile int _prndindex;
	
	/**
	 * Initializes the Doom random number generator.
	 *
	 * @since 2016/05/07
	 */
	public DoomRandom()
	{
	}
	
	/**
	 * Returns the current pseudo-random index.
	 *
	 * @return The current pseudo-random index.
	 * @since 2016/06/07
	 */
	public int getIndex()
	{
		return _prndindex & 0xFF;
	}
	
	/**
	 * Returns the next pseudo random number.
	 *
	 * @return The next pseudo random number.
	 * @since 2016/06/07
	 */
	public int next()
	{
		return _RANDOM_TABLE[(++_prndindex) & 0xFF];
	}
	
	/**
	 * Returns the next signed pseudo random number.
	 *
	 * @return The next signed pseudo random number.
	 * @since 2016/06/07
	 */
	public int nextSigned()
	{
		return next() - next();
	}
	
	/**
	 * Resets the random number index to zero.
	 *
	 * @return {@code this}.
	 * @since 2016/05/07
	 */
	public DoomRandom resetIndex()
	{
		return setIndex(0);
	}
	
	/**
	 * Sets the pseudo random index.
	 *
	 * @param __dx The index to set.
	 * @return {@code this}.
	 * @since 2016/06/07
	 */
	public DoomRandom setIndex(int __dx)
	{
		_prndindex = __dx & 0xFF;
		return this;
	}
}

