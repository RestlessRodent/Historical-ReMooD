// -*- Mode: Java; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-
// ----------------------------------------------------------------------------
// ReMooD Doom Source Port <http://remood.org/>
//   Copyright (C) 2005-2015 GhostlyDeath <ghostlydeath@remood.org>
//     For more credits, see AUTHORS.
// ----------------------------------------------------------------------------
// ReMooD is under the GNU General Public License v3 (or later), see COPYING.
// ----------------------------------------------------------------------------

package org.remood.remood;

import static java.lang.System.err;

/**
 * Main game entry class.
 */
public class Main
{
	/** Major version number. */
	public static final int MAJOR =
		1;
	
	/** Minor version number. */
	public static final int MINOR =
		0;
	
	/** Release version code. */
	public static final char RELEASE =
		'a';
	
	/** Full version string. */
	public static final String FULL =
		"" + MAJOR + "." + MINOR + RELEASE;
	
	/**
	 * Main entry method.
	 *
	 * @param __args Game arguments.
	 */
	public static void main(String __args[])
	{
		// Print version
		err.printf("ReMooD %s%n", FULL);
	}
}

