// -*- Mode: Java; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-
// ---------------------------------------------------------------------------
// ReMooD Doom Source Port <http://remood.org/>
//   Copyright (C) 2005-2016 GhostlyDeath <ghostlydeath@remood.org>
//     For more credits, see readme.mkd.
// ---------------------------------------------------------------------------
// ReMooD is under the GNU General Public License v3+, see license.mkd.
// ---------------------------------------------------------------------------

package org.remood.remood.core.profile;

import java.nio.file.Path;

/**
 * This is the base class for all profiles which are saved on the local system.
 *
 * @since 2016/05/10
 */
public abstract class SavedProfile
	extends Profile
{
	/** The location where the profile is saved. */
	private volatile Path _savepath;
	
	/**
	 * Initializes an unsaved local profile.
	 *
	 * @since 2016/05/19
	 */
	public SavedProfile()
	{
		// Not saved
		_savepath = null;
	}
	
	/**
	 * Initializes the local profile.
	 *
	 * @param __p The location where the profile is saved.
	 * @throws NullPointerException On null arguments.
	 * @since 2016/05/19
	 */
	public SavedProfile(Path __p)
		throws NullPointerException
	{
		if (__p == null)
			throw new NullPointerException();
		
		// Is saved
		_savepath = __p;
	}
	
	/**
	 * Returns the path that the profile is saved to.
	 *
	 * @return The path of the profile.
	 * @since 2016/05/19
	 */
	public Path getPath()
	{
		// Lock
		synchronized (lock)
		{
			return _savepath;
		}
	}
	
	/**
	 * Returns whether or not the profile is saved to the disk.
	 *
	 * @return {@code true} if the profile is saved.
	 * @since 2016/05/19
	 */
	public boolean isSaved()
	{
		// Lock
		synchronized (lock)
		{
			return _savepath != null;
		}
	}
	
	/**
	 * Sets the path where the profile is saved.
	 *
	 * @param __p The save location of the profile.
	 * @throws NullPointerException On null arguments.
	 * @since 2016/05/19
	 */
	public void setPath(Path __p)
		throws NullPointerException
	{
		// Check
		if (__p == null)
			throw new NullPointerException();
		
		// Set
		synchronized (lock)
		{
			_savepath = __p;
		}
	}
	
	/**
	 * Sets that this profile is not to be saved and clears the save path.
	 *
	 * @since 2016/05/19
	 */
	public void setUnsaved()
	{
		// Lock
		synchronized (lock)
		{
			_savepath = null;
		}
	}
}

