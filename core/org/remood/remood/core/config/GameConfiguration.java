// -*- Mode: Java; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-
// ---------------------------------------------------------------------------
// ReMooD Doom Source Port <http://remood.org/>
//   Copyright (C) 2005-2016 GhostlyDeath <ghostlydeath@remood.org>
//     For more credits, see readme.mkd.
// ---------------------------------------------------------------------------
// ReMooD is under the GNU General Public License v3+, see license.mkd.
// ---------------------------------------------------------------------------

package org.remood.remood.core.config;

import org.remood.remood.core.config.CommandLineArguments;
import org.remood.remood.core.console.GameConsole;

/**
 * This class handles management of the game configuration and all of the
 * desired settings.
 *
 * @since 2016/07/17
 */
public class GameConfiguration
{
	/** The game console. */
	protected final GameConsole console;
	
	/**
	 * Initializes the configuration.
	 *
	 * @param __gc The game console.
	 * @param __cl The command line.
	 * @throws NullPointerException On null arguments.
	 * @since 2016/07/17
	 */
	public GameConfiguration(GameConsole __gc, CommandLineArguments __cl)
		throws NullPointerException
	{
		// Check
		if (__gc == null || __cl == null)
			throw new NullPointerException();
		
		// Set
		this.console = __gc;
	}
}

