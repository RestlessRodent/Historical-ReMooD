// -*- Mode: Java; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-
// ---------------------------------------------------------------------------
// ReMooD Doom Source Port <http://remood.org/>
//   Copyright (C) 2005-2016 GhostlyDeath <ghostlydeath@remood.org>
//     For more credits, see readme.mkd.
// ---------------------------------------------------------------------------
// ReMooD is under the GNU General Public License v3+, see license.mkd.
// ---------------------------------------------------------------------------

package org.remood.remood.core.console;

import java.io.PrintStream;

/**
 * This contains the core console which is linked to an input command buffer
 * and an output.
 *
 * @since 2016/07/16
 */
public class GameConsole
{
	/** The input console which is used for command parsing. */
	public final PrintStream in =
		new PrintStream(new __InputCommandConsole__(this));
	
	/** The output console which is displayed to the user. */
	public final PrintStream out =
		new PrintStream(new __OutputCommandConsole__(this, System.out));
	
	/**
	 * Initializes the console.
	 *
	 * @since 2016/07/16
	 */
	public GameConsole()
	{
	}
}

