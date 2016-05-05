// -*- Mode: Java; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-
// ---------------------------------------------------------------------------
// ReMooD Doom Source Port <http://remood.org/>
//   Copyright (C) 2005-2016 GhostlyDeath <ghostlydeath@remood.org>
//     For more credits, see readme.mkd.
// ---------------------------------------------------------------------------
// ReMooD is under the GNU General Public License v3+, see license.mkd.
// ---------------------------------------------------------------------------

package org.remood.remood.core;

/**
 * This contains the ReMooD version information.
 *
 * @since 2016/05/05
 */
public final class Version
{
	/** Major version. */
	public static final int MAJOR_VERSION =
		1;
	
	/** Minor version. */
	public static final int MINOR_VERSION =
		0;
	
	/** Release version. */
	public static final char RELEASE_VERSION =
		'a';
	
	/** Code name of this version. */
	public static final String CODE_NAME =
		"Stuffed Cabbage";
	
	/** Full display string. */
	public static final String FULL_DISPLAY_STRING =
		"ReMooD " + MAJOR_VERSION + "." + MINOR_VERSION + RELEASE_VERSION +
		" \"" + CODE_NAME + "\"";
	
	/**
	 * Do not call this.
	 *
	 * @since 2016/05/05
	 */
	private Version()
	{
	}
}

