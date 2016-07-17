// -*- Mode: Java; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-
// ---------------------------------------------------------------------------
// ReMooD Doom Source Port <http://remood.org/>
//   Copyright (C) 2005-2016 GhostlyDeath <ghostlydeath@remood.org>
//     For more credits, see readme.mkd.
// ---------------------------------------------------------------------------
// ReMooD is under the GNU General Public License v3+, see license.mkd.
// ---------------------------------------------------------------------------

package org.remood.remood.core.config;

/**
 * This class contains some default command line arguments which may be used
 * to initialize the game or setup some other state (such as playing a demo).
 *
 * Arguments are either switches which start with a dash, or console commands
 * to be executed which start with a plus. Unlike previously some arguments
 * now accept multiple options (such as demo playback).
 *
 * @since 2016/07/16
 */
public class CommandLineArguments
{
	/**
	 * This parses the command line arguments which may be used when parsing
	 * the configuration file.
	 *
	 * @param __args Program arguments.
	 * @throws NullPointerException On null arguments.
	 * @since 2016/07/16
	 */
	public CommandLineArguments(String... __args)
		throws NullPointerException
	{
		// Check
		if (__args == null)
			throw new NullPointerException();
		
		throw new Error("TODO");
	}
}

