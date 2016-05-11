// -*- Mode: Java; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-
// ---------------------------------------------------------------------------
// ReMooD Doom Source Port <http://remood.org/>
//   Copyright (C) 2005-2016 GhostlyDeath <ghostlydeath@remood.org>
//     For more credits, see readme.mkd.
// ---------------------------------------------------------------------------
// ReMooD is under the GNU General Public License v3+, see license.mkd.
// ---------------------------------------------------------------------------

package org.remood.remood.core.profile;

import java.lang.ref.Reference;
import java.lang.ref.WeakReference;
import java.nio.file.Path;
import java.nio.file.Paths;

/**
 * This class manages profiles which are basically user accounts that may be
 * used by the game.
 *
 * @since 2016/05/10
 */
public class ProfileManager
{
	/** The root directory where profiles are stored. */
	protected final Path root;
	
	/**
	 * Initializes the profile manager which uses the given directory as the
	 * root directory which contains profiles to be used by the game.
	 *
	 * @param __r The root directory where profiles are stored.
	 * @throws NullPointerException On null arguments.
	 * @since 2016/05/10
	 */
	public ProfileManager(Path __r)
		throws NullPointerException
	{
		// Check
		if (__r == null)
			throw new NullPointerException("NARG");
		
		// Set
		root = __r;
		
		System.err.printf("DEBUG -- Profile root: %s%n", __r);
	}
	
	/**
	 * Initializes the profile manager which uses the given directory as the
	 * root directory which contains profiles to be used by the game.
	 *
	 * @param __r The root directory where profiles are stored.
	 * @throws NullPointerException On null arguments.
	 * @since 2016/05/11
	 */
	public ProfileManager(String __s)
		throws NullPointerException
	{
		this(Paths.get(__s));
	}
}

